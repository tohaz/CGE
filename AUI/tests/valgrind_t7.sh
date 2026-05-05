make
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./bin/t7_window 2>&1 | tee valgrind_output_t7.lst

