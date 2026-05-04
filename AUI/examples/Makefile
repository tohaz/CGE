# --- Universal Makefile for Examples/Tests (Sub-directory) ---
AUI_ROOT   := ..
BUILD_MODE ?= debug

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
# Using += allows users to override this from the command line if needed
MAKEFLAGS += -j$(NPROCS)

# Define library and build directory paths based on BUILD_MODE
ifeq ($(BUILD_MODE), debug)
  AUI_LIB_DIR := $(AUI_ROOT)/Custom
  BUILD_DIR   := bin
else
  AUI_LIB_DIR := $(AUI_ROOT)/Custom/$(BUILD_MODE)
  BUILD_DIR   := bin/$(BUILD_MODE)
endif

AUI_LIB := $(AUI_LIB_DIR)/libAUI.a
DEP_DIR := $(BUILD_DIR)/deps
SRC_DIR := .

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

# Source and Target discovery
SRCS := $(wildcard $(SRC_DIR)/*.cpp)
EXES := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%,$(SRCS))
DEPS := $(patsubst $(SRC_DIR)/%.cpp,$(DEP_DIR)/%.d,$(SRCS))

# Main target: Build all executables
all: $(EXES)

# Mode switching targets
release srelease:
	@$(MAKE) BUILD_MODE=$@ all

# Rule to build the library file. 
# Linking $(EXES) will wait for this command to finish completely.
$(AUI_LIB):
	@echo "--- Ensuring AUI Library [Mode: $(BUILD_MODE)] ---"
	@$(MAKE) -C $(AUI_ROOT) BUILD_MODE=$(BUILD_MODE) all

# Rule for building applications.
# $(AUI_LIB) is a prerequisite, preventing race conditions during linking.
$(BUILD_DIR)/%: $(SRC_DIR)/%.cpp $(AUI_LIB) | $(BUILD_DIR) $(DEP_DIR)
	@echo "Building application [$(BUILD_MODE)]: $<"
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -MF $(DEP_DIR)/$*.d $< -o $@ $(LDFLAGS) $(LIBS)
ifneq ($(BUILD_MODE), debug)
	@$(STRIP) --strip-all $@
endif

# Ensure build directories exist before compilation starts
$(BUILD_DIR) $(DEP_DIR):
	@mkdir -p $@

# Include automatically generated dependency files (.d)
-include $(DEPS)

clean:
	@echo "--- Cleaning Sub-project [$(BUILD_MODE)] ---"
	rm -rf $(BUILD_DIR)
	@$(MAKE) -C $(AUI_ROOT) BUILD_MODE=$(BUILD_MODE) clean

.PHONY: all clean release srelease