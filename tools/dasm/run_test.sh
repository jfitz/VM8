TEST=cpm_kernel

echo test $TEST

# make test dir
mkdir testbed
mkdir testbed/$TEST

# copy files

echo running 'dasm'

# copy files
cp tests/$TEST/stdin.rel testbed/$TEST

# run test, generate output
ruby dasm.rb <testbed/$TEST/stdin.rel -o opcodes_8080.t >testbed/$TEST/stdout.txt

echo comparing output

# compare output against expected output
echo
echo stdout
diff testbed/$TEST/stdout.txt tests/$TEST/stdout.txt
cp testbed/$TEST/stdout.txt tests/$TEST/stdout.txt
