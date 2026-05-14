make
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./bin/t10_messagebox 2>&1 | tee valgrind_output_t10.lst

