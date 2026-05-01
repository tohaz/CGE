#valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose ./cg0 2>&1 | tee valgrind_output.lst
stdbuf -o0 valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./cg0 2>&1 | tee valgrind_output.lst
#valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./cg0

