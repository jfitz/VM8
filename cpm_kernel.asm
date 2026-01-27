# CPM kernel

# call handler for function in C
DISPATCH
    LXI H,TABLE
    CPI C,TABLE_CT
    JP  CMD_ERR
    MVI B,0
    DAD B
    PCHL

CMD_ERR
    RET

# program exit
C_EXIT
    RET

# console input char
CON_C_IN
    RET

# console output char
CON_C_OUT
    OUT 1
    RET

# console output line
CON_S_OUT
    OUT 9
    RET

TABLE
.word	C_EXIT      # 0x00
.word	CON_C_IN    # 0x01
.word	CON_C_OUT   # 0x02
.word	0           # 0x03
.word	0           # 0x04
.word	0           # 0x05
.word	0           # 0x06
.word	0           # 0x07
.word	0           # 0x08
.word	CON_S_OUT   # 0x09

.equate TABLE_SZ    .address TABLE -
.equate TABLE_CT    TABLE_SZ 2 /

