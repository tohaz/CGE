# --- Paths ---
SRC_DIR := .
OBJ_DIR := Custom/obj
LIB_DIR := Custom

# --- Compiler Settings ---
CXX := g++
AR := ar

# -Wold-style-cast -Weffc++ 
# 
#CXXFLAGS := -Wall -Wextra -MMD -MP -I. -std=c++26 -O0 -g3 -pedantic-errors -Werror -Wshadow -Wfloat-equal -Wswitch-default -Wcast-align
CXXFLAGS := -fmax-errors=15 -std=c++26 -O0 -g3 -pedantic -pedantic-errors -Wall -Wextra -Werror -Wcast-align -Wcast-qual -Wdisabled-optimization -Wlogical-op -Wmissing-declarations -Wmissing-include-dirs -Wnoexcept \
			-Woverloaded-virtual -Wredundant-decls -Wshadow -Wsign-promo -Wstrict-null-sentinel -Wswitch-default -Wundef -Wfloat-equal -Wconversion -Wsign-conversion -c 
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

