.equate bdos   0x0005   # BDOS entry point
start   MVI    C,9      # BDOS function: output string
        LXI    D,msg    # address of msg
        CALL   bdos
        RET             # return to CCP

.string	msg 'Hello, world!$'


