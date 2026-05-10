require 'optparse'
require '../tools.rb'

def make_opcode_descs(file_lines)
  descriptions = {}
  
  file_lines.each do |line|
    line.strip!
    next if line.empty?

    # split on comment; take only first part
    split_line = line.split("#")

    desc_text = ''
    desc_text = split_line[0] if split_line.size > 0

    if desc_text.size > 0
      parts = desc_text.split
      opcode_s = parts.shift
      opcode = opcode_s.to_i(0)
      desc = parts.join(' ')
    
      descriptions[opcode] = desc
    end
  end
  
  descriptions
end

def disassemble(offset, opcode_desc, code_bytes, references, output_base)
  parts = opcode_desc.split
  mnemonic = parts.shift
  mnemonic = mnemonic.ljust(4)
  opcode_text = mnemonic + '   ' + parts.join

  ref_offset = offset + 1

  if opcode_desc.end_with?('byte')
    arg_value_s = format_byte(code_bytes[ref_offset], output_base)

    # if address is in references, replace with symbol
    arg_value_s = references[ref_offset] if references.key?(ref_offset)

    opcode_text.sub!('byte', arg_value_s)
  end

  if opcode_text.end_with?('word')
    arg_value_l = code_bytes[ref_offset]
    arg_value_h = code_bytes[ref_offset + 1]

    arg_value = arg_value_h * 256 + arg_value_l
    arg_value_s = format_word(arg_value, output_base)

    # if address is in references, replace with symbol
    arg_value_s = references[ref_offset] if references.key?(ref_offset)

    opcode_text.sub!('word', arg_value_s)
  end

  opcode_text
end

options = {}
OptionParser.new do |opts|
  opts.banner = "Usage: ruby dasm.rb [options]"

  opts.on("-o", "--opcodes NAME", "File name for opcodes table") do |v|
    options[:opcodes_name] = v
  end

  opts.on("--octal", "Octal output") do |v|
    options[:octal] = v
  end

  opts.on("--hex", "Hexadecimal output") do |v|
    options[:hex] = v
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

output_base = :octal
output_base = :hex if options[:hex]

file_lines = File.readlines(opcodes_filename, chomp: true)
sections = split_into_sections(file_lines)

processor = nil

if sections.key?('.environment')
  env_lines = sections['.environment']
  
  environment = make_dictionary(env_lines)
  
  processor = environment['processor']
end

if !sections.key?('.opcodes')
  puts "no opcodes section"
  exit
end

opcodes_lines = sections['.opcodes']
opcode_descriptions = make_opcode_descs(opcodes_lines)

# at this point we no longer need 'file_lines' and 'sections'
file_lines = STDIN.readlines(chomp: true)
sections = split_into_sections(file_lines)

# verify .identification
unless sections.key?('.identification')
  STDERR.puts 'No header'
  exit
end

bytes = []

lines = sections['.executable']

if lines.nil?
  STDERR.puts 'No executable or code section'
  exit
end

begin
  code_bytes = parse_bytes(lines)
rescue RuntimeError => e
  STDERR.puts e
end

lines = sections['.instruction-offsets']

begin
  instruction_offsets = parse_words(lines)
rescue RuntimeError => e
  STDERR.puts e
end

references_lines = sections['.references']
references = make_references_map(references_lines)

# for each instruction offset
instruction_offsets.each do |offset|
  # get the opcode
  op = code_bytes[offset]
  
  # get the description of the opcode
  opcode_desc = opcode_descriptions[op]

  # build the text description
  if opcode_desc.nil?
    opcode_text = "No opcode for #{op_s} / #{op}"
  else
    opcode_text = disassemble(offset, opcode_desc, code_bytes, references, output_base)
  end

  # print the disassembled code  
  s = ''
  s += format_word(offset, output_base)
  s += ": "
  s += format_byte(op, output_base)
  # space
  # byte or 4 spaces
  # space
  # byte or 4 spaces
  s += '  '
  s += opcode_text
  
  puts s
end

# write end
puts '.end'

