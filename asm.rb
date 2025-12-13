require 'optparse'

def make_opcodes_table(filename)
  opcodes = {}

  File.foreach(filename) do |line|
    # split on #
    parts = line.split('#')
    next if parts[0].size == 0

    text = parts[0].chomp
    next if text.empty?

    words = text.split

    # must have 2 or 3
    if words.count > 3
      puts 'Bad opcode spec: ' + text
      exit
    end

    mnemonic = words[0]
    # [0] must start with alpha
    char = mnemonic[0]
    unless char.match?(/[A-Za-z]/)
      puts 'Bad mnemonic: ' + mnemonic
      exit
    end
    
    # check no duplicate
    if opcodes.key?(mnemonic)
      puts 'Duplicate mnemonic: ' + mnemonic
      exit
    end

    opcode_text = words[1]

    # [1] must be numeric (octal, hex, dec)
    opcode = opcode_text.to_i(0)

    # [2] if exists must be numeric
    argcount = words[2].to_i || 0

    # store
    opcodes[mnemonic] = { 'op' => opcode, 'sz' => argcount }
  end
  
  opcodes
end

options = {}
OptionParser.new do |opts|
  opts.banner = "Usage: ruby my_app_options.rb [options]"

  opts.on("-o", "--opcodes NAME", "File name for opcodes table") do |v|
    options[:opcodes_name] = v
  end

  opts.on("-l", "--labels NAME", "File name for label values") do |v|
    options[:labels_name] = v
  end

  opts.on("-v", "--[no-]verbose", "Run verbosely") do |v|
    options[:verbose] = v
  end

  opts.on("-h", "--help", "Prints this help") do
    puts opts
    exit
  end
end.parse!

opcodes_table_filename = options[:opcodes_name]

if opcodes_table_filename.nil?
  puts "opcodes table required"
  exit
end

opcodes_table = make_opcodes_table(opcodes_table_filename)

verbose = options[:verbose]
address = 0

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

  asm_text = ':' + asm_text if asm_text.match(/^\s/) 
  items = asm_text.split
  
  label = ''
  label = items[0] if items.size > 0 && items[0] != ':'

  items.shift
  big_mnemonic = items.join
  mnems = big_mnemonic.split(',')
  big_mnemonic = mnems[0]
  arg_text = ''
  arg_value = 0
  if mnems.size > 1
    arg_text = mnems[1]
    arg_value = 0 # do lookup in pass 2
  end

  mnemonic = ''
  mnemonic = big_mnemonic if items.size > 0
  
  opcode_spec = opcodes_table[mnemonic] || { 'op' => 0, 'sz' => 0 } 
  opcode = opcode_spec['op']
  arg_size = opcode_spec['sz'] || 0
  
  if mnemonic.size > 0
    if opcode.nil?
      puts 'Unknown mnemonic: ' + big_mnemonic
    else
      print "%#06o" % address + ': ' if verbose
      print "%#03o " % opcode
      print "%#03o " % 0 if arg_size > 0
      print "%#03o " % 0 if arg_size > 1
      print '# ' + label + ' ' + big_mnemonic
      print ', ' + arg_text if arg_text.size > 0
      
      address += 1
      address += arg_size
    end
  end

  print ' #' + comment if comment.size > 0
  puts
end
