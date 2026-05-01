CXX = g++
CXXFLAGS = -Wall -Wextra -g -std=c++20
INCLUDE_PATH = ../
CPPFLAGS = -I$(INCLUDE_PATH)
LDFLAGS = -Llib -L../Custom/
LIBS = -lCGLib -lX11 -lX11-xcb -lxcb -lxcb-cursor

SRC_DIR = ./
BUILD_DIR = ./bin/

SRCS = $(wildcard $(SRC_DIR)*.cpp)

EXES = $(patsubst $(SRC_DIR)%.cpp,$(BUILD_DIR)%,$(SRCS))

all: clean $(EXES)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
	make -C ..

$(BUILD_DIR)%: $(SRC_DIR)%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $< -o $@ $(LDFLAGS) $(LIBS)

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)