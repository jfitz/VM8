TEST=$1

echo test $TEST

# make test dir
mkdir testbed/$TEST

# copy files

echo running 'loader'

# copy files
cp tests/$TEST/stdin.txt testbed/$TEST

# run test, generate output
ruby loader.rb <testbed/$TEST/stdin.txt --end 0xffff >testbed/$TEST/stdout.txt

echo comparing output

# compare output against expected output
echo
echo stdout
diff testbed/$TEST/stdout.txt tests/$TEST/stdout.txt
cp testbed/$TEST/stdout.txt tests/$TEST/stdout.txt
