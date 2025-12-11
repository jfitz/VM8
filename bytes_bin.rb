def to_bin(text)
  return 0 if text.empty?
  
  return text.to_i(16) if text.start_with?('0x')
  
  return text.to_i(8) if text.start_with?('0')
  
  return text.to_i
end

# open output in binary mode
File.open("filename.bin", "wb") do |output|
  # for each line in input
  while line = gets
    chomped = line.chomp

    # split on comment; take only first part
    split_line = chomped.split("#")

    next if split_line.empty?

    # split on spaces
    values = split_line[0].split

    values.each do |value|
      puts value
      # convert each item to binary
      c = to_bin(value)

      # emit binary
      output.write([c].pack('c'))
    end
  end
end

