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

def read_and_split_lines
  sections = {}

  section = []
  name = ''

  $stdin.each_line do |line|
    line.chomp!
    line.strip!
    if line.start_with?('.')
      # a dot-line starts a new section
      sections[name] = section if name.size > 0
      section = []
      name = line
    else
      # other lines get added to section
      section << line
    end
  end

  sections[name] = section if section.size > 0

  sections
end

# option for base

options = {}
OptionParser.new do |opts|
  opts.banner = "Usage: ruby loader.rb [options]"

  opts.on("-b", "--base BASE", "Base address for this module") do |v|
    options[:base_address] = v
  end

  opts.on("-h", "--help", "Prints this help") do
    puts opts
    exit
  end
end.parse!

base_address = 0
base_address_s = options[:base_address]
base_address = base_address_s.to_i(0) unless base_address_s.nil?

puts "base-address: #{base_address}"

# split input to sections based on dot lines
sections = read_and_split_lines

# verify .relocatable
unless sections.key?('.relocatable')
  STDERR.puts 'No header'
  exit
end

# verify .code
unless sections.key?('.code')
  STDERR.puts 'No code section'
  exit
end

bytes = []

sections['.code'].each do |line|
  parts = line.split

  parts.each do |byte|
    bytes << byte.to_i(0)
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

# adjust symbols by base
symbols.each do |_, symbol_def|
  symbol_def.adjust(base_address) if symbol_def.is_rel
end

# adjust references in code
references.each do |offset, reference_def|
  name = reference_def.symbol
  num_bytes = reference_def.num_bytes
  symbol_def = symbols[name]

  if symbol_def.nil?
    STDERR.puts 'Symbol not found: ' + name
    exit
  end

  target = symbol_def.offset
  bytes[offset] = target % 256 if num_bytes > 0
  bytes[offset + 1] = target / 256 if num_bytes > 1
end

# write header
puts '.executable'

# write bytes
bytes.each do |byte|
  byte_s = format_octal_byte(byte)

  puts byte_s
end

# write end
puts '.end'
