valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./bin/t5_table 2>&1 | tee valgrind_output_t5.lst

