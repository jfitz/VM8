// 'create vm1 Intel8080' create a virtual machine for 8080

// 'opcodes' emit the opcode file

// 'load cpm_kernel.bin 0xf380' load cpm kernel into high memory
// 'pc 0xf380' set PC to cpm kernel init function
// no stack
// 'run' run until HLT, inits CP/M

// 'dump 0x00' to examine memory and verify init operation

// 'load hello.com 0x0100' load user program into TPA
// 'sp 0xf380' set SP to start of CPM kernel (stack grows downward)
// 'push 0x0000' onto stack (so a RET goes to CP/M exit API call)

// 'pc 0x100'

// 'debug'
// 'step' until RET to 0

// 'destroy vm1' destroy virtual machine
// 'quit'
