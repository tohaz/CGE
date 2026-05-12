make
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./bin/t9_popup_menu 2>&1 | tee valgrind_output_t9.lst

