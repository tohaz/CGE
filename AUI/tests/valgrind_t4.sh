stdbuf -o0 valgrind --leak-check=full --track-origins=yes --show-leak-kinds=all --track-origins=yes ./bin/t4_label 2>&1 | tee valgrind_output_t4.lst

