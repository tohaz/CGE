make
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./bin/t0_aui 2>&1 | tee valgrind_output_t0.lst

