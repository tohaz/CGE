valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./bin/t2_list 2>&1 | tee valgrind_output_t2.lst

