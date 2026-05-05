# --- AUI Library Makefile (Root) ---
SRC_DIR := .
BUILD_MODE ?= debug

# Detect number of CPU cores
NPROCS := 1
OS := $(shell uname -s)
ifeq ($(OS),Linux)
  NPROCS := $(shell nproc)
endif
ifeq ($(OS),Darwin)
  NPROCS := $(shell sysctl -n hw.ncpu)
endif

# Enable multithreading
MAKEFLAGS += -j$(NPROCS)

# Directory logic: debug stays in Custom, others in Custom/mode
ifeq ($(BUILD_MODE), debug)
  LIB_DIR := Custom
else
  LIB_DIR := Custom/$(BUILD_MODE)
endif

OBJ_DIR := $(LIB_DIR)/obj

CXX := g++
AR  := ar

# Strict compiler flags
BASE_FLAGS := -fmax-errors=15 -std=c++26 -pedantic -pedantic-errors -Wall -Wextra -Werror \
              -Wcast-align -Wcast-qual -Wdisabled-optimization -Wlogical-op \
              -Wmissing-declarations -Wmissing-include-dirs -Wnoexcept \
              -Woverloaded-virtual -Wredundant-decls -Wshadow -Wsign-promo \
              -Wstrict-null-sentinel -Wswitch-default -Wundef -Wfloat-equal \
              -Wconversion -Wsign-conversion -MMD -MP

GIT_VERSION := $(shell git describe --always --dirty --tags 2>/dev/null || echo "unknown")
BASE_FLAGS  += -DAUI_GIT_VERSION=\"$(GIT_VERSION)\"              
              

# Optimization logic based on BUILD_MODE
ifeq ($(BUILD_MODE), release)
  CXXFLAGS := $(BASE_FLAGS) -O3
else ifeq ($(BUILD_MODE), srelease)
  CXXFLAGS := $(BASE_FLAGS) -Os -ffunction-sections -fdata-sections
else
  CXXFLAGS := $(BASE_FLAGS) -O0 -g3
endif

SRC_FILES  := $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES  := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRC_FILES))
LIB_NAME   := libAUI.a
STATIC_LIB := $(LIB_DIR)/$(LIB_NAME)

# --- Rules ---

# Default target: The library is the main goal
all: $(STATIC_LIB)

# Mode switching targets
release srelease:
	@$(MAKE) BUILD_MODE=$@ all

# Archive the library: All OBJ_FILES must be finished first
$(STATIC_LIB): $(OBJ_FILES)
	@echo "--- Archiving AUI Library [Mode: $(BUILD_MODE)] ---"
	$(AR) rcs $@ $(OBJ_FILES)

# Compile objects:
# Uses order-only dependency '| $(OBJ_DIR)' to ensure directory exists 
# BEFORE compilation starts, without relying on timestamps.
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	@echo "Compiling AUI [$(BUILD_MODE)]: $<"
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Create mode-specific directories
$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

# Include auto-generated dependencies
-include $(OBJ_FILES:.o=.d)

# Clean only the current mode
clean:
	@echo "--- Cleaning AUI Library [$(BUILD_MODE)] ---"
	rm -rf $(OBJ_DIR) $(STATIC_LIB)
	@if [ "$(BUILD_MODE)" != "debug" ]; then rm -rf $(LIB_DIR); fi

.PHONY: all clean release srelease


