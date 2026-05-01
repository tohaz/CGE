# Define directories and variables
SRC_DIR := ./
INCLUDE_DIR := ./
OBJ_DIR := Custom/obj
LIB_DIR := Custom
#BIN_DIR := Custom

CXX := g++
AR := ar
CXXFLAGS := -Wall -Wextra -std=c++20 
LDFLAGS := -L$(LIB_DIR)                       
LIBS := -lCGLib         

# Files
SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRC_FILES))
LIB_NAME := libCGLib.a
STATIC_LIB := $(LIB_DIR)/$(LIB_NAME)

# Default target
all: dirs $(STATIC_LIB)

# Create necessary directories
dirs:
	mkdir -p $(OBJ_DIR) $(LIB_DIR) $(BIN_DIR)

# Rule to create the static library
$(STATIC_LIB): $(OBJ_FILES)
	$(AR) rcs $@ $(OBJ_FILES)

# Rule to compile source files into object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up generated files
clean:
	rm -rf $(OBJ_DIR) $(LIB_DIR) $(BIN_DIR)

.PHONY: all clean dirs