# --- Universal Makefile for Examples/Tests (Sub-directory) ---
AUI_ROOT   := ..
BUILD_MODE ?= debug

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
  CXXFLAGS := $(BASE_FLAGS) -O0 -g3
  LDFLAGS  := -L$(AUI_LIB_DIR)
endif

CPPFLAGS := -I$(AUI_ROOT)
LIBS     := -lAUI -lX11 -lX11-xcb -lxcb -lxcb-cursor

SRCS := $(wildcard $(SRC_DIR)/*.cpp)
EXES := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%,$(SRCS))
DEPS := $(patsubst $(SRC_DIR)/%.cpp,$(DEP_DIR)/%.d,$(SRCS))

all: build_aui_lib $(EXES)

release srelease:
	@$(MAKE) BUILD_MODE=$@ all

.PHONY: build_aui_lib
build_aui_lib:
	@echo "--- Ensuring AUI Library [Mode: $(BUILD_MODE)] ---"
	@$(MAKE) -C $(AUI_ROOT) BUILD_MODE=$(BUILD_MODE) all

$(BUILD_DIR)/%: $(SRC_DIR)/%.cpp $(AUI_LIB) | $(BUILD_DIR) $(DEP_DIR)
	@echo "Building application [$(BUILD_MODE)]: $<"
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -MF $(DEP_DIR)/$*.d $< -o $@ $(LDFLAGS) $(LIBS)
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

.PHONY: all clean build_aui_lib release srelease
