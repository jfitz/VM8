# CP/M kernel
	NOP
start
label	NOP
	INX B

	LXI H, TPA	# start of user program
label2	HLT
loop	MVI A,
	MVI A,5
	JMP loop
	
