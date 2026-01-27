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
.dw C_EXIT      # 0x00
.dw CON_C_IN    # 0x01
.dw CON_C_OUT   # 0x02
.dw 0           # 0x03
.dw 0           # 0x04
.dw 0           # 0x05
.dw 0           # 0x06
.dw 0           # 0x07
.dw 0           # 0x08
.dw CON_S_OUT   # 0x09

.equate TABLE_SZ    .address TABLE -
.equate TABLE_CT    TABLE_SZ 2 /

