valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./bin/t6_helpers 2>&1 | tee valgrind_output_t6.lst

