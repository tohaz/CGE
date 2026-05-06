# 
# Additional make targets:
# 1. Release optimized by speed (make release) 
# 2. Release optimized by size (make srelease)
#

# --- Universal Makefile for a single Binary (Sub-directory) ---
AUI_ROOT   := ../../AUI
BUILD_MODE ?= debug

# Name of your final output binary
TARGET_NAME := cg0

# Detect number of CPU cores for parallel build
NPROCS := 1
OS := $(shell uname -s)
ifeq ($(OS),Linux)
  NPROCS := $(shell nproc)
endif
ifeq ($(OS),Darwin)
  NPROCS := $(shell sysctl -n hw.ncpu)
endif

# Enable multithreading by default
MAKEFLAGS += -j$(NPROCS)

# Automatically delete target if a rule fails
.DELETE_ON_ERROR:

# Define library and build directory paths based on BUILD_MODE
ifeq ($(BUILD_MODE), debug)
  AUI_LIB_DIR := $(AUI_ROOT)/Custom
  BUILD_DIR   := bin
else
  AUI_LIB_DIR := $(AUI_ROOT)/Custom/$(BUILD_MODE)
  BUILD_DIR   := bin/$(BUILD_MODE)
endif

AUI_LIB  := $(AUI_LIB_DIR)/libAUI.a
OBJ_DIR  := $(BUILD_DIR)/obj
DEP_DIR  := $(BUILD_DIR)/deps
SRC_DIR  := .
TARGET   := $(BUILD_DIR)/$(TARGET_NAME)

CXX      := g++
STRIP    := strip

# Compilation flags
BASE_FLAGS := -std=c++26 -MMD -pedantic -pedantic-errors -Wall -Wextra -Werror \
              -Wcast-align -Wdisabled-optimization -Wlogical-op -Wmissing-include-dirs \
              -Wfloat-equal -Wconversion -Wsign-conversion

ifeq ($(BUILD_MODE), release)
  CXXFLAGS := $(BASE_FLAGS) -O3
  LDFLAGS  := -L$(AUI_LIB_DIR)
else ifeq ($(BUILD_MODE), srelease)
  CXXFLAGS := $(BASE_FLAGS) -Os -ffunction-sections -fdata-sections
  LDFLAGS  := -L$(AUI_LIB_DIR) -Wl,--gc-sections
else
  CXXFLAGS := $(BASE_FLAGS) -O0 -g3
  LDFLAGS  := -L$(AUI_LIB_DIR)
endif

CPPFLAGS := -I$(AUI_ROOT)
LIBS     := -lAUI -lX11 -lX11-xcb -lxcb -lxcb-cursor

# Source and Object discovery
SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))
DEPS := $(patsubst $(SRC_DIR)/%.cpp,$(DEP_DIR)/%.d,$(SRCS))

# Main target: Build the single binary
all: $(TARGET)

# Mode switching
release srelease:
	@$(MAKE) BUILD_MODE=$@ all

# Rule to build the external library
$(AUI_LIB):
	@echo "--- Ensuring AUI Library [Mode: $(BUILD_MODE)] ---"
	@$(MAKE) -C $(AUI_ROOT) BUILD_MODE=$(BUILD_MODE) all

# Linking rule: Builds the final binary from all object files
$(TARGET): $(OBJS) $(AUI_LIB) | $(BUILD_DIR)
	@echo "Linking binary [$(BUILD_MODE)]: $@"
	@rm -f $@
	$(CXX) $(OBJS) -o $@ $(LDFLAGS) $(LIBS) || (rm -f $@ && exit 1)
ifneq ($(BUILD_MODE), debug)
	@$(STRIP) --strip-all $@
endif

# Compilation rule: Compile each .cpp into an .o file
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR) $(DEP_DIR)
	@echo "Compiling: $<"
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -MF $(DEP_DIR)/$*.d -c $< -o $@

# Create necessary directories
$(BUILD_DIR) $(OBJ_DIR) $(DEP_DIR):
	@mkdir -p $@

# Include auto-generated dependency files
-include $(DEPS)

clean:
	@echo "--- Cleaning Sub-project [$(BUILD_MODE)] ---"
	rm -rf $(BUILD_DIR)
	@$(MAKE) -C $(AUI_ROOT) BUILD_MODE=$(BUILD_MODE) clean

.PHONY: all clean release srelease