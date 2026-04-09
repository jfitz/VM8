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

echo comparing output

# compare output against expected output
echo
echo stdout
diff testbed/$TEST/stdout.txt tests/$TEST/stdout.txt
cp testbed/$TEST/stdout.txt tests/$TEST/stdout.txt

echo
echo list
diff testbed/$TEST/list.txt tests/$TEST/list.txt
cp testbed/$TEST/list.txt tests/$TEST/list.txt
