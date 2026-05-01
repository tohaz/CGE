valgrind --tool=callgrind --dump-instr=yes --collect-jumps=yes ./cg0
#callgrind_annotate $(ls -t callgrind.out.* | head -n1) > ./callgrind.lst
kcachegrind $(ls -t callgrind.out.* | head -n1)
rm -rf ./callgrind.out.*

