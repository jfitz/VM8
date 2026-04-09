TEST=$1

echo test $TEST

# make test dir
mkdir testbed/$TEST

# copy files

echo running 'asm'

# copy files
cp tests/$TEST/stdin.asm testbed/$TEST

# run test, generate output
ruby asm.rb -o opcodes_8080.t -l testbed/$TEST/list.txt <testbed/$TEST/stdin.asm >testbed/$TEST/stdout.txt

ruby asm.rb -o opcodes_8080.t -l testbed/$TEST/list_octal.txt --octal <testbed/$TEST/stdin.asm >testbed/$TEST/stdout_octal.txt

ruby asm.rb -o opcodes_8080.t -l testbed/$TEST/list_hex.txt --hex <testbed/$TEST/stdin.asm >testbed/$TEST/stdout_hex.txt

echo comparing output

# compare output against expected output
echo
echo stdout
diff testbed/$TEST/stdout.txt tests/$TEST/stdout.txt
cp testbed/$TEST/stdout.txt tests/$TEST/stdout.txt
diff testbed/$TEST/stdout_octal.txt tests/$TEST/stdout_octal.txt
cp testbed/$TEST/stdout_octal.txt tests/$TEST/stdout_octal.txt
diff testbed/$TEST/stdout_hex.txt tests/$TEST/stdout_hex.txt
cp testbed/$TEST/stdout_hex.txt tests/$TEST/stdout_hex.txt

echo
echo list
diff testbed/$TEST/list.txt tests/$TEST/list.txt
cp testbed/$TEST/list.txt tests/$TEST/list.txt
diff testbed/$TEST/list_octal.txt tests/$TEST/list_octal.txt
cp testbed/$TEST/list_octal.txt tests/$TEST/list_octal.txt
diff testbed/$TEST/list_hex.txt tests/$TEST/list_hex.txt
cp testbed/$TEST/list_hex.txt tests/$TEST/list_hex.txt

