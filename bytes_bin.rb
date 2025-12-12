require 'optparse'

options = {}
OptionParser.new do |opts|
  opts.banner = "Usage: ruby my_app_options.rb [options]"

  opts.on("-o", "--output NAME", "Specify output file") do |v|
    options[:output_filename] = v
  end

  opts.on("-v", "--[no-]verbose", "Run verbosely") do |v|
    options[:verbose] = v
  end

  opts.on("-h", "--help", "Prints this help") do
    puts opts
    exit
  end
end.parse!

output_filename = options[:output_filename] || 'output.bin'

# open output in binary mode
File.open(output_filename, "wb") do |output|
  # for each line in input
  while line = gets
    chomped = line.chomp

    # split on comment; take only first part
    split_line = chomped.split("#")

    next if split_line.empty?

    # split on spaces
    values = split_line[0].split

    values.each do |value|
      print ' ' + value if options[:verbose]
      # convert each item to binary
      c = value.to_i(0)

      # emit binary
      output.write([c].pack('c'))
    end
    
    puts if options[:verbose]
  end
end

