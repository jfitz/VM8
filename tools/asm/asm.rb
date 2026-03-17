require 'optparse'
require '../tools.rb'

class AsmError < RuntimeError; end

class AbsRelValue
  attr_reader :value, :is_rel

  def initialize(value, is_rel)
    # value is numeric
    # is_rel.is_a?(TrueClass) || is_rel.is_a?(FalseClass)

    @value = value
    @is_rel = is_rel
  end

  def two_bytes
    [@value % 256, @value / 256]
  end
end

def make_opcodes_defs(filename)
  opcode_defs = {}

  File.foreach(filename) do |line|
    # split on #
    parts = line.split('#')
    next if parts[0].size == 0

    text = parts[0].chomp.strip
    next if text.empty?

    words = text.split("\t")

    # must have 2
    if words.count != 2
      puts 'Bad opcode spec: ' + text
      exit
    end

    # opcode is first on line
    opcode_text = words[0]

    # [1] must be numeric (octal, hex, dec)
    opcode = opcode_text.to_i(0)

    tokens_1 = words[1].chomp.split(/([\s,])/)
    tokens_2 = tokens_1.map(&:strip)
    tokens = tokens_2.reject(&:empty?)

    # store
    curr_node = opcode_defs

    tokens.each do |token|
      if curr_node.class.to_s != 'Hash'
        puts 'Collision'
        exit
      end

      unless curr_node.key?(token)
        curr_node[token] = {}
      end

      curr_node = curr_node[token]
    end

    curr_node['op'] = opcode
  end
  
  opcode_defs
end

def extract_literals(node, literals)
  node.each do |text, subtree|
    # only the text items; omit comma and numbers
    literals[text] = 0 if text.match(/^[A-Z]+$/)

    extract_literals(subtree, literals) if subtree.class.to_s == 'Hash'
  end
end

def make_known_literals(opcode_defs)
  known_literals = {}

  extract_literals(opcode_defs, known_literals)
  
  known_literals.keys
end

def find_opcode(node, tokens)
  arg_size = 0
  arg_tokens = []
  mnemonic = []
  
  tokens.each do |token|
    arg_size = 2 if token == 'address'
    arg_size = 1 if token == 'byte'

    # possibly change case of token here
    if node.key?(token)
      # the node matches the token
      node = node[token]
      mnemonic << token
    elsif node.key?('address')
      # an address in def requires an expression list in tokens
      if token.class.to_s == 'Array'
        node = node['address']
        arg_tokens = token
      else
        STDERR.puts 'cannot find ' + '[' + tokens.join(' ') + ']'
        exit
      end
    elsif node.key?('byte')
      # a byte in def requires an expression list in tokens
      if token.class.to_s == 'Array'
        node = node['byte']
        arg_tokens = token
      else
        STDERR.puts 'cannot find ' + '[' + tokens.join(' ') + ']'
        exit
      end
    else
      STDERR.puts 'cannot find ' + '[' + tokens.join(' ') + ']'
      exit
    end
  end

  unless node.key?('op')
    puts 'cannot find opcode'
    exit
  end

  return node['op'], mnemonic, arg_tokens
end

def parse_directive_line(asm_text)
  # does not allow for quoted space chars or strings
  parts = asm_text.split
  
  directive = parts.shift
  
  return directive, parts
end

def pack_tokens(tokens, known_literals)
  packed_tokens = []

  expression = []

  tokens.each do |token|
    if known_literals.include?(token)
      if expression.size > 0
        packed_tokens << expression
        expression = []
      end
      
      packed_tokens << token
    elsif token == ','
      if expression.size > 0
        packed_tokens << expression
        expression = []
      end
      
      packed_tokens << token
    else
      expression << token
    end
  end

  packed_tokens << expression if expression.size > 0
  
  packed_tokens
end

# find opcode def that matches the input line
def parse_asm_line(asm_text, opcode_defs, known_literals)
  # force a first item for the split
  asm_text = ':' + asm_text if asm_text.match(/^\s/) 
  tokens = asm_text.split(/[\s\,]/).reject(&:empty?)
  tokens_1 = asm_text.chomp.split(/([\s,])/)
  tokens_2 = tokens_1.map(&:strip)
  tokens = tokens_2.reject(&:empty?)
  
  # drop the forced item to make 'label' empty string
  label = nil
  label = tokens[0] if tokens.size > 0 && tokens[0] != ':'

  tokens.shift   # remove label

  opcode = nil
  mnemonic = []
  arg_size = 0
  arg_tokens = []

  if tokens.size > 0
    # change expressions to array
    packed_tokens = pack_tokens(tokens, known_literals)
    opcode, mnemonic, arg_tokens = find_opcode(opcode_defs, packed_tokens)
  end

  return label, opcode, mnemonic, arg_tokens
end

def format_generated_bytes(opcode, arg_size, arg_value)
  s = format_octal_byte(opcode)
  
  s += ' '
  
  # print LSB of arg
  if arg_size > 0
    s += format_octal_byte(arg_value % 256)
  else
    s += format_octal_byte(nil)
  end
    
  s += ' '

  # print MSB of arg    
  if arg_size > 1
    s += format_octal_byte(arg_value / 256)
  else
    s += format_octal_byte(nil)
  end
  
  s
end

def format_asm_line(label, mnemonic, arg_tokens)
  s = '# '
  
  if label.nil?
    s += '        '
  else
    s += label.ljust(8)
  end

  # first item, tab, other items
  if mnemonic.size > 0
    printed_mnemonic = mnemonic.shift

    if mnemonic.size > 0 || arg_tokens.size > 0
      space_count = 6 - printed_mnemonic.size
      spaces = ' ' * space_count
      printed_mnemonic += spaces
    end
    
    if mnemonic.size > 0
      printed_mnemonic += mnemonic.join
    end
  end

  s += printed_mnemonic

  unless arg_tokens.empty?
    s += arg_tokens.join(' ')
  end
  
  s
end

def format_output(offset, opcode, arg_size, arg_value, label, mnemonic, arg_text)
  s = ''

  # print offset
  s += format_octal_word(offset) + ': '

  # print generated bytes
  s += format_generated_bytes(opcode, arg_size, arg_value)
    
  # print source line
  s += ' '
  s += format_asm_line(label, mnemonic, arg_text)
end

def format_nongen_output(label, comment, offset)
  s = ''
  
  if label.nil?
    # no label means no offset, just spaces (and only then if a comment)
    if comment.size > 0
      s += '         '
      s += '               #'
    end
  else
    # print offset and label
    s += format_octal_word(offset) + ': '
    s += '               # ' + label
  end
  
  s
end

def format_bytes_output(offset, values)
  s = ''
  
  # print offset
  if offset.nil?
    s += '       : '
  else
    s += format_octal_word(offset) + ': '
  end

  # print generated bytes
  values.each do |value|
    s += format_octal_byte(value)
    s += ' '
  end
  
  s
end

options = {}
OptionParser.new do |opts|
  opts.banner = "Usage: ruby asm.rb [options]"

  opts.on("-o", "--opcodes NAME", "File name for opcodes table") do |v|
    options[:opcodes_name] = v
  end

  opts.on("-l", "--list NAME", "File name for list output") do |v|
    options[:list_name] = v
  end

  opts.on("-h", "--help", "Prints this help") do
    puts opts
    exit
  end
end.parse!

opcodes_filename = options[:opcodes_name]

if opcodes_filename.nil?
  puts "opcodes file required"
  exit
end

opcodes_defs = make_opcodes_defs(opcodes_filename)

known_literals = make_known_literals(opcodes_defs)

list_output_filename = options[:list_name]

bytes = []
symbols = {}
references = {}
instr_offs = []

offset = 0
word_count = 0

list_lines = []

# for each line in input
while line = gets
  chomped = line.chomp
  next if line.empty?

  # split on comment; take only first part
  split_line = chomped.split("#")

  asm_text = ''
  asm_text = split_line[0] if split_line.size > 0

  comment = ''
  comment = split_line[1] if split_line.size > 1

  list_line = ''

  unless asm_text.empty?
    if asm_text[0] == '.'
      # process directive
      directive, parts = parse_directive_line(asm_text)
      
      case directive
      when '.equate'
        # parse label, expression (tokens separated by whitespace)
        label = parts.shift

        # evaluate expression in RPN
        expression = RpnExpression.new(parts)
        values = expression.evaluate(offset, symbols)
        raise AsmError('invalid expression ' + parts.join(' ')) if values.nil?
        value = values[0]
        byte_values = value.two_bytes
        bytes_s = format_octal_word(byte_values)
        list_line = '         ' + bytes_s + "\t# " + directive + '  ' + label + '  ' + parts.join(' ')

        # store value in equates table (check for inconsistency)
        symbols[label] = value
      when '.word'
        instr_offs << offset

        # parse expression
        # evaluate parts as RPN
        expression = RpnExpression.new(parts)
        values = expression.evaluate(offset, symbols)
        raise AsmError('invalid expression ' + parts.join(' ')) if values.nil?
        value = values[0]
        # gen 2 bytes (low byte first)
        byte_values = value.two_bytes
        bytes << byte_values[0]
        bytes << byte_values[1]
        
        bytes_s = format_bytes_output(offset, byte_values)
        list_line = bytes_s + "\t# " + directive + '  ' + parts.join(' ')

        if value.is_rel
          if parts.size == 1
            label = parts[0]
          else
            label = 'word_' + word_count.to_s
          end

          symbols[label] = value
          reference_def = ReferenceDef.new(label, 2)
          references[offset] = reference_def
        end

        offset += values.size * 2
        word_count += 1
      else
        puts 'unknown directive: ' + directive
        exit
      end
    else
      # process code
      label, opcode, mnemonic, arg_tokens = parse_asm_line(asm_text, opcodes_defs, known_literals)

      symbols[label] = AbsRelValue.new(offset, true) unless label.nil?

      arg_value = 0
      arg_size = arg_tokens.count

      if opcode.nil?
        list_line = format_nongen_output(label, comment, offset)
      else
        # offset to this instruction
        instr_offs << offset

        # store the opcode
        bytes << opcode

        arg_value = 0

        # evaluate argument
        if arg_tokens.size > 0
          expression = RpnExpression.new(arg_tokens)
          arg_values = expression.evaluate(offset, symbols) || [AbsRelValue.new(0, true)]
          arg_value0 = arg_values[0]
          arg_value = arg_value0.value

          # store argument byte or bytes
          bytes << arg_value % 256 if arg_size > 0
          bytes << arg_value / 256 if arg_size > 1

          arg_text = arg_tokens.join
          
          if arg_size > 0
            if arg_text.match(/\A[A-Z][A-Z0-9_]*\z/)
              offset_ref = offset + 1  # skip over the opcode
              reference_def = ReferenceDef.new(arg_text, arg_size)
              references[offset_ref] = reference_def
            end
          end
        end

        # add to code segment
        list_line = format_output(offset, opcode, arg_size, arg_value, label, mnemonic, arg_tokens)

        offset += 1
        offset += arg_size
      end
    end
  end

  # print comment (if any)
  list_line += ' #' + comment if comment.size > 0

  list_lines << list_line
end

# write list output
unless list_output_filename.nil?
  File.open(list_output_filename, "w") do |file|
    list_lines.each do |line|
      file.puts(line)
    end
  end
end

# write relocatable module
puts '.relocatable'

puts '.code'
bytes.each do |byte|
  byte_s = format_octal_byte(byte)

  puts byte_s
end

puts '.instructions'
instr_offs.each do |offset|
  offset_s = format_octal_word(offset)

  puts offset_s
end

puts '.symbols'

symbols.each do |symbol, abs_rel_value|
  value_s = format_octal_word(abs_rel_value.value)
  abs_or_rel = "abs"
  abs_or_rel = "rel" if abs_rel_value.is_rel

  puts symbol + "\t" + value_s + "\t" + abs_or_rel
end

puts '.references'

references.each do |offset, reference_def|
  offset_s = format_octal_word(offset)

  puts offset_s + "\t" + reference_def.to_s
end

puts '.end'
