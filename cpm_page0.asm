# code for CP/M page 0

.include cpm_defs.a

.address 0x0000

	HLT
	NOP
	NOP
	NOP
	NOP

BDOS_E	OUT	1
	RET


