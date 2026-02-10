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

def op_add(a, b)
  # abs + abs => abs
  # abs + rel => rel
  # rel + abs => rel
  # rel + rel => error

  raise Exception('two rel values for plus') if a.is_rel && b.is_rel

  is_rel = false
  is_rel = true if a.is_rel
  is_rel = true if b.is_rel

  AbsRelValue.new(a.value + b.value, is_rel)
end

def op_subtract(a, b)
  # abs - abs => abs
  # abs - rel => rel
  # rel - abs => rel
  # rel - rel => abs

  is_rel = false
  is_rel = true if a.is_rel != b.is_rel

  AbsRelValue.new(a.value - b.value, is_rel)
end

def op_multiply(a, b)
  # abs * abs => abs
  # abs * rel => error
  # rel * abs => error
  # rel * rel => error

  raise Exception('two rel values for multiply') if a.is_rel || b.is_rel

  is_rel = false

  AbsRelValue.new(a.value * b.value, is_rel)
end

def op_divide(a, b)
  # check for divide by zero
  # abs / abs => abs
  # abs / rel => error
  # rel / abs => error
  # rel / rel => error

  raise Exception('two rel values for divide') if a.is_rel || b.is_rel

  is_rel = false

  AbsRelValue.new(a.value / b.value, is_rel)
end

def eval_rpn(tokens, offset, symbols)
  # return a list of values
  values = []
  
  tokens.each do |token|
    case token
    when '+'
      # check at least two values
      # pop two values
      b = values.pop
      a = values.pop
      # execute
      result = op_add(a, b)
      # push result
      values << result
    when '-'
      # check at least two values
      # pop two values
      b = values.pop
      a = values.pop
      # execute
      result = op_subtract(a, b)
      # push result
      values << result
    when '*'
      # check at least two values
      # pop two values
      b = values.pop
      a = values.pop
      # execute
      result = op_multiply(a, b)
      # push result
      values << result
    when '/'
      # check at least two values
      # pop two values
      b = values.pop
      a = values.pop
      # execute
      result = op_divide(a, b)
      # push result
      values << result
    when /^\d+$/
      # convert
      value = token.to_i(0)
      # push
      values << AbsRelValue.new(value, false)
    when /\A[A-Z][A-Z0-9_]*\z/
      # look up value
      value = symbols[token]
      # push
      values << value
    when '.offset'
      values << AbsRelValue.new(offset, true)
    else
      puts "unknown token: '" + token + "'"
    end
  end
  
  values
end
