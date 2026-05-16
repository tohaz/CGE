make
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./bin/t12_progressbar 2>&1 | tee valgrind_output_t11.lst

