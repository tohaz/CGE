# 
# Additional make targets:
# 1. Release optimized by speed(make release) 
# 2. Release, optimized by size(make srelease)
#

# --- Universal Makefile for Examples/Tests (Sub-directory) ---
AUI_ROOT   := ..
BUILD_MODE ?= debug
# [FEATURE] Enable multithreading
MAKEFLAGS += -j$(shell nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 1)

# Synchronize library path logic with AUI root Makefile
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
  # [FEATURE] Enhanced debug flags for Valgrind
  CXXFLAGS := $(BASE_FLAGS) -O0 -g3 -fno-inline -fno-omit-frame-pointer
  LDFLAGS  := -L$(AUI_LIB_DIR)
endif

CPPFLAGS := -I$(AUI_ROOT)
LIBS     := -lAUI -lX11 -lX11-xcb -lxcb -lxcb-cursor

SRCS := $(wildcard $(SRC_DIR)/*.cpp)
EXES := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%,$(SRCS))
DEPS := $(patsubst $(SRC_DIR)/%.cpp,$(DEP_DIR)/%.d,$(SRCS))

# --- Rules ---

# [FEATURE] Added $(AUI_LIB) as prerequisite to ensure check & link if library changes
all: build_aui_lib $(EXES)

release srelease:
	@$(MAKE) BUILD_MODE=$@ all

.PHONY: build_aui_lib
build_aui_lib:
	@echo "--- Ensuring AUI Library [Mode: $(BUILD_MODE)] ---"
	@$(MAKE) -C $(AUI_ROOT) BUILD_MODE=$(BUILD_MODE) all

# [FEATURE] Added $(AUI_LIB) to the dependency list and rm -f for safety
$(BUILD_DIR)/%: $(SRC_DIR)/%.cpp $(AUI_LIB) | $(BUILD_DIR) $(DEP_DIR)
	@echo "Building application [$(BUILD_MODE)]: $<"
	@rm -f $@
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -MF $(DEP_DIR)/$*.d $< -o $@ $(LDFLAGS) $(LIBS) || (rm -f $@ && exit 1)
ifneq ($(BUILD_MODE), debug)
	@$(STRIP) --strip-all $@
endif

$(BUILD_DIR) $(DEP_DIR):
	@mkdir -p $@

-include $(DEPS)

clean:
	@echo "--- Cleaning Sub-project [$(BUILD_MODE)] ---"
	rm -rf $(BUILD_DIR)
	@$(MAKE) -C $(AUI_ROOT) BUILD_MODE=$(BUILD_MODE) clean

# [FEATURE] Use FORCE to check library without triggering unnecessary rebuilds
FORCE:
$(AUI_LIB): FORCE
	@$(MAKE) -C $(AUI_ROOT) BUILD_MODE=$(BUILD_MODE) all

.PHONY: all clean build_aui_lib release srelease
