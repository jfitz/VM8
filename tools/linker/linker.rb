require 'optparse'
require '../tools.rb'

class SymbolDef
  attr_reader :offset, :is_rel

  def initialize(offset, is_rel)
    @offset = offset
    @is_rel = is_rel
  end

  def adjust(adjustment)
    @offset += adjustment
  end

  def to_s
    offset_s = format_octal_word(@offset)
    "#{offset_s}, #{@is_rel}"
  end
end

# option for base

options = {}
OptionParser.new do |opts|
  opts.banner = "Usage: ruby linker.rb [options]"

  opts.on("-h", "--help", "Prints this help") do
    puts opts
    exit
  end
end.parse!

# split input to sections based on dot lines
file_lines = STDIN.readlines(chomp: true)
sections = split_into_sections(file_lines)

# verify .relocatable
unless sections.key?('.identification')
  STDERR.puts 'No header'
  exit
end

processor = nil

if sections.key?('.environment')
  env_lines = sections['.environment']
  
  environment = make_dictionary(env_lines)
  
  processor = environment['processor']
end

# verify .executable
unless sections.key?('.executable')
  STDERR.puts 'No executable section'
  exit
end

executable_bytes = []
readonly_bytes = []
writable_bytes = []

sections['.executable'].each do |line|
  parts = line.split

  parts.each do |byte|
    executable_bytes << byte.to_i(0)
  end
end

# skip over instruction offsets

# verify .symbols
unless sections.key?('.symbols')
  STDERR.puts 'No symbols section'
  exit
end

symbols = {}

sections['.symbols'].each do |line|
  parts = line.split

  unless parts.size == 3
    STDERR.puts 'Bad symbol def: ' + line
    exit
  end

  offset = parts[1].to_i(0)
  is_rel = parts[2] == 'rel'
  symbols[parts[0]] = SymbolDef.new(offset, is_rel)
end

# verify .references
unless sections.key?('.references')
  STDERR.puts 'No references section'
  exit
end

references = {}

sections['.references'].each do |line|
  parts = line.split

  unless parts.size == 3
    STDERR.puts 'Bad reference def: ' + line
    exit
  end

  offset = parts[0].to_i(0)
  name = parts[1]
  num_bytes = parts[2].to_i(0)
  references[offset] = ReferenceDef.new(name, num_bytes)
end

# for each segment, adjust symbols by base of segment
base_address = 0

symbols.each do |_, symbol_def|
  symbol_def.adjust(base_address) if symbol_def.is_rel
end

base_address = executable_bytes.count

# adjust readonly segment

base_address = executable_bytes.count + readonly_bytes.count

# adjust writable segment

executable_relocation_offsets = []

# adjust references in code
references.each do |offset, reference_def|
  name = reference_def.symbol
  num_bytes = reference_def.num_bytes
  symbol_def = symbols[name]

  if symbol_def.nil?
    STDERR.puts 'Symbol not found: ' + name
    exit
  end

  # add to table
  executable_relocation_offsets << offset if symbol_def.is_rel

  # adjust bytes in segment
  target = symbol_def.offset

  executable_bytes[offset] = target % 256 if num_bytes > 0
  executable_bytes[offset + 1] = target / 256 if num_bytes > 1
end

# write relocatable module
puts '.identification'
puts 'relocatable'

puts '.environment'

puts "processor\t" + processor unless processor.nil?

# write executable segment
puts '.executable'

executable_bytes.each do |byte|
  byte_s = format_octal_byte(byte)

  puts byte_s
end

# write readonly segment

# write writable segment

# write executable-relocation-offsets section

puts ".relocation-offsets"

executable_relocation_offsets.each do |offset|
  puts format_octal_word(offset)
end

# write end
puts '.end'
