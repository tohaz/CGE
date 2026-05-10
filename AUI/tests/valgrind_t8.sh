make
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./bin/t8_widget 2>&1 | tee valgrind_output_t8.lst

