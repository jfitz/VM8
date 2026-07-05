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

  def to_s(output_base)
    offset_s = format_word(@offset, output_base)
    "#{offset_s}, #{@is_rel}"
  end
end

# option for base

options = {}
OptionParser.new do |opts|
  opts.banner = "Usage: ruby linker.rb [options]"

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

output_base = :octal
output_base = :hex if options[:hex]

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
lines = sections['.executable']

if lines.nil?
  STDERR.puts 'No executable section'
  exit
end

begin
  executable_bytes = parse_bytes(lines)
rescue RuntimeError => e
  STDERR.puts e
end

readonly_bytes = []
writable_bytes = []

# skip over instruction offsets

# verify .symbols
unless sections.key?('.symbols')
  STDERR.puts 'No symbols section'
  exit
end

symbols = {}

lines = sections['.symbols']

lines.each do |line|
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
  num_bytes = parts[1].to_i(0)
  name = parts[2]
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
write_bytes(executable_bytes, output_base)

# write readonly segment

# write writable segment

# write executable-relocation-offsets section

puts ".relocation-offsets"
write_words(executable_relocation_offsets, output_base)

# write end
puts '.end'
