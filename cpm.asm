# CP/M kernel
	NOP
label	NOP
	INX B
	LXI H, TPA	# start of user program
	HLT
	MVI A,
	MVI A,5
	
