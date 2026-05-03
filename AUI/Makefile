# --- Paths (Relative) ---
SRC_DIR := .
OBJ_DIR := Custom/obj
LIB_DIR := Custom

# --- Compiler Settings ---
CXX := g++
AR := ar
CXXFLAGS := -Wall -Wextra -MMD -MP -I. -std=c++26 -O0 -g3 -fmessage-length=0 -pedantic -pedantic-errors -Werror

# --- Files ---
SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRC_FILES))
LIB_NAME  := libCGLib.a
STATIC_LIB := $(LIB_DIR)/$(LIB_NAME)

# --- Rules ---

all: dirs $(STATIC_LIB)

dirs:
	mkdir -p $(OBJ_DIR)

$(STATIC_LIB): $(OBJ_FILES)
	$(AR) rcs $@ $(OBJ_FILES)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Include dependencies (.d files)
-include $(OBJ_FILES:.o=.d)

clean:
	rm -rf $(LIB_DIR)

.PHONY: all clean dirs

