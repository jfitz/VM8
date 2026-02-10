TEST=cpm_kernel

echo test $TEST

# make test dir
mkdir testbed
mkdir testbed/$TEST

# copy files

echo running 'linker'

# copy files
cp tests/$TEST/stdin.rel testbed/$TEST

# run test, generate output
ruby ../linker.rb <testbed/$TEST/stdin.rel --base 0x100 >testbed/$TEST/stdout.txt

echo comparing output

# compare output against expected output
echo
echo stdout
diff testbed/$TEST/stdout.txt tests/$TEST/stdout.txt
cp testbed/$TEST/stdout.txt tests/$TEST/stdout.txt
