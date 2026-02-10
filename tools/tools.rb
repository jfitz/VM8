def format_octal_byte(n)
  s = ''
  
  unless n.nil?
    s = ("%#03o" % n)
  end
  
  s.rjust(4)
end

def format_octal_word(n)
  s = ''
  
  unless n.nil?
    s = ("%#06o" % n)
  end
  
  s.rjust(7)
end

class ReferenceDef
  attr_reader :symbol, :num_bytes

  def initialize(symbol, num_bytes)
    @symbol = symbol
    @num_bytes = num_bytes
  end

  def to_s
    "#{@symbol}\t#{@num_bytes}"
  end
end

