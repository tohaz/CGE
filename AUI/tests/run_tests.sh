#!/bin/bash

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

rm -rf ./*.lst

make

valgrind --error-exitcode=1 --leak-check=full --errors-for-leak-kinds=all ./bin/t1_button
if [ $? -eq 0 ]; then
    echo -e "${GREEN}+++Test 1 Success+++${NC}"
else
    echo -e "${RED}Failed Test1==========================================================================================${NC}"
		exit 1
fi

valgrind --error-exitcode=1 --leak-check=full --errors-for-leak-kinds=all ./bin/t2_list
if [ $? -eq 0 ]; then
    echo -e "${GREEN}+++Test 2 Success+++${NC}"
else
    echo -e "${RED}Failed Test2==========================================================================================${NC}"
		exit 1
fi

valgrind --error-exitcode=1 --leak-check=full --errors-for-leak-kinds=all ./bin/t3_inputbox
if [ $? -eq 0 ]; then
    echo -e "${GREEN}+++Test 3 Success+++${NC}"
else
    echo -e "${RED}Failed Test3==========================================================================================${NC}"
		exit 1
fi

valgrind --error-exitcode=1 --leak-check=full --errors-for-leak-kinds=all ./bin/t4_label
if [ $? -eq 0 ]; then
    echo -e "${GREEN}+++Test 4 Success+++${NC}"
else
    echo -e "${RED}Failed Test4==========================================================================================${NC}"
		exit 1
fi

valgrind --error-exitcode=1 --leak-check=full --errors-for-leak-kinds=all ./bin/t5_table
if [ $? -eq 0 ]; then
    echo -e "${GREEN}+++Test 5 Success+++${NC}"
else
    echo -e "${RED}Failed Test5==========================================================================================${NC}"
		exit 1
fi

valgrind --error-exitcode=1 --leak-check=full --errors-for-leak-kinds=all ./bin/t6_helpers
if [ $? -eq 0 ]; then
    echo -e "${GREEN}+++Test 6 Success+++${NC}"
else
    echo -e "${RED}Failed Test6==========================================================================================${NC}"
		exit 1
fi

valgrind --error-exitcode=1 --leak-check=full --errors-for-leak-kinds=all ./bin/t7_window
if [ $? -eq 0 ]; then
    echo -e "${GREEN}+++Test 7 Success+++${NC}"
else
    echo -e "${RED}Failed Test7==========================================================================================${NC}"
		exit 1
fi

valgrind --error-exitcode=1 --leak-check=full --errors-for-leak-kinds=all ./bin/t8_widget
if [ $? -eq 0 ]; then
    echo -e "${GREEN}+++Test 8 Success+++${NC}"
else
    echo -e "${RED}Failed Test8==========================================================================================${NC}"
		exit 1
fi

valgrind --error-exitcode=1 --leak-check=full --errors-for-leak-kinds=all ./bin/t9_popup_menu
if [ $? -eq 0 ]; then
    echo -e "${GREEN}+++Test 9 Success+++${NC}"
else
    echo -e "${RED}Failed Test9==========================================================================================${NC}"
		exit 1
fi

echo -e "${GREEN}+++ALL Success+++${NC}"
