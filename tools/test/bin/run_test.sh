TEST=cpm_kernel

echo test $TEST

# make test dir
mkdir tests
mkdir tests/$TEST

# copy files

echo running 'asm'

# run test, generate output
ruby asm.rb -o opcodes_8080.tsv <test/data/$TEST.asm >tests/$TEST/out.txt

echo comparing output

# compare output against expected output
diff tests/$TEST/out.txt test/ref/$TEST.txt
