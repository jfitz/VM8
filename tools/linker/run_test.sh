TEST=cpm_kernel

echo test $TEST

# make test dir
mkdir testbed
mkdir testbed/$TEST

# copy files

echo running 'linker'

# copy files
cp tests/$TEST/stdin.txt testbed/$TEST

# run test, generate output
ruby linker.rb <testbed/$TEST/stdin.txt --end 0xffff >testbed/$TEST/stdout.txt

echo comparing output

# compare output against expected output
echo
echo stdout
diff testbed/$TEST/stdout.txt tests/$TEST/stdout.txt
cp testbed/$TEST/stdout.txt tests/$TEST/stdout.txt
