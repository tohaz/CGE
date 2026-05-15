make
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./bin/t11_combobox 2>&1 | tee valgrind_output_t11.lst

