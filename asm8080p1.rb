def make_opcodes_table
  opcodes =
  {
    'NOP' => 0,
    'LXIB' => 1,
    'STAXB' => 2,
    'INXB' => 3,
    'INRB' => 4,
    'DCRB' => 5,
    'MVIB' => 6,
    'RLC' => 7,

    'DADB' => 011,
    'LDAXB' => 012,
    'DCXB' => 013,
    'INRC' => 014,
    'DCRC' => 015,
    'MVIC' => 016,
    'RRC' => 017,

    'LXID' => 021,
    'STAXD' => 022,
    'INXD' => 023,
    'INRD' => 024,
    'DCRD' => 025,
    'MVID' => 026,
    'RAL' => 027,

    'DADD' => 031,
    'LDAXD' => 032,
    'DCXD' => 033,
    'INRE' => 034,
    'DCRE' => 035,
    'MVIE' => 036,
    'RAR' => 037,

    'LXIH' => 041,
    'SHLD' => 042,
    'INXH' => 043,
    'INRH' => 044,
    'DCRH' => 045,
    'MVIH' => 046,
    'DAA' => 047,

    'DADH' => 051,
    'LHLD' => 052,
    'DCXH' => 053,
    'INRL' => 054,
    'DCRL' => 055,
    'MVIL' => 056,
    'CMA' => 057,

    'LXISP' => 061,
    'STA' => 062,
    'INXSP' => 063,
    'INRM' => 064,
    'DCRM' => 065,
    'MVIM' => 066,
    'STC' => 067,

    'DADSP' => 071,
    'LDA' => 072,
    'DCXSP' => 073,
    'INRA' => 074,
    'DCRA' => 075,
    'MVIA' => 076,
    'CMC' => 077,


    'HLT' => 0114
  }
end

opcodes_table = make_opcodes_table

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
  arg = ''
  if mnems.size > 1
    arg = mnems[1]
  end

  mnemonic = ''
  mnemonic = big_mnemonic if items.size > 0
  
  opcode = 0
  opcode = opcodes_table[mnemonic] if opcodes_table.key?(mnemonic)
  
  if mnemonic.size > 0
    if opcode.nil?
      puts 'Unknown mnemonic: ' + big_mnemonic
    else
      print "%#03o" % opcode + ' # '
      print label + ' ' + big_mnemonic
      print ', ' + arg if arg.size > 0
      print '# ' + comment if comment.size > 0
      puts
    end
  else
    print '# ' + comment if comment.size > 0
    puts
  end
end
