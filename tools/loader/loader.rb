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

  opts.on("-b", "--start ADDRESS", "Start address") do |v|
    options[:start_address] = v
  end

  opts.on("-e", "--end ADDRESS", "End address") do |v|
    options[:end_address] = v
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

base_address_s = options[:start_address]
end_address_s = options[:end_address]

if !base_address_s.nil? && !end_address_s.nil?
  STDERR.puts 'Conflicting start and end address options. Pick only one.'
  exit
end

base_address = nil
base_address = base_address_s.to_i(0) unless base_address_s.nil?

end_address = nil
end_address = end_address_s.to_i(0) unless end_address_s.nil?

base_address = 0 if base_address.nil? && end_address.nil?

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

# verify .relocation-offsets
lines = sections['.relocation-offsets']

if lines.nil?
  STDERR.puts 'No relocation offsets section'
  exit
end

begin
  offsets = parse_words(lines)
rescue RuntimeError => e
  STDERR.puts e
end

# start specified, no end specified
if !base_address.nil? && end_address.nil?
  end_address = base_address + executable_bytes.count
end

# no start specified, end specified
if base_address.nil? && !end_address.nil?
  base_address = end_address - executable_bytes.count
end

puts 'base-address ' + format_word(base_address, output_base)
puts 'end-address ' + format_word(end_address, output_base)

# adjust references in code
offsets.each do |offset|
  lsb = executable_bytes[offset]
  msb = executable_bytes[offset + 1]
  address = msb * 256 + lsb
  
  address += base_address
  
  msb = address / 256
  lsb = address % 256
  
  executable_bytes[offset] = lsb
  executable_bytes[offset + 1] = msb
end

# write positioned module
puts '.identification'
puts 'positioned'

puts '.environment'

puts "processor\t" + processor unless processor.nil?

# write executable segment
puts '.executable'
write_bytes(executable_bytes, output_base)

# write end
puts '.end'
