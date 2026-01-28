TEST=cpm_kernel

echo test $TEST

# make test dir
mkdir tests
mkdir tests/$TEST

# copy files

echo running 'asm'

# run test, generate output
ruby asm.rb -o opcodes_8080.tsv -l tests/$TEST/list.out <test/data/$TEST.asm >tests/$TEST/asm.out

echo comparing output

# compare output against expected output
diff tests/$TEST/asm.out test/ref/$TEST.txt
