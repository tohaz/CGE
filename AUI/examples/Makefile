# --- Paths (Relative) ---
LIB_ROOT   := ..
CGLIB_PATH := $(LIB_ROOT)/Custom/libCGLib.a

SRC_DIR    := .
# Default build mode is debug (options: debug, release, srelease)
BUILD_MODE ?= debug

# Set build directory based on mode
ifeq ($(BUILD_MODE), release)
    BUILD_DIR := bin/release
else ifeq ($(BUILD_MODE), srelease)
    BUILD_DIR := bin/srelease
else
    BUILD_DIR := bin
endif

DEP_DIR    := $(BUILD_DIR)/deps

# --- Tools ---
CXX      := g++
STRIP    := strip

# --- Compiler Settings ---
# Base strict flags for code quality and standards compliance
BASE_FLAGS := -std=c++26 -MMD -pedantic -pedantic-errors -Wall -Wextra -Werror \
              -Wcast-align -Wdisabled-optimization -Wlogical-op -Wmissing-include-dirs \
              -Wnoexcept -Woverloaded-virtual -Wredundant-decls -Wshadow -Wsign-promo \
              -Wstrict-null-sentinel -Wswitch-default -Wundef -Wfloat-equal

# Mode-specific flags
ifeq ($(BUILD_MODE), release)
    # Optimization for speed
    CXXFLAGS := $(BASE_FLAGS) -O3
    LDFLAGS  := -L$(LIB_ROOT)/Custom
else ifeq ($(BUILD_MODE), srelease)
    # Maximum optimization for size
    # -Os: optimize for size
    # -ffunction-sections -fdata-sections: place each function/data item into its own section
    CXXFLAGS := $(BASE_FLAGS) -Os -ffunction-sections -fdata-sections
    # -Wl,--gc-sections: tell the linker to remove unused sections (dead code stripping)
    LDFLAGS  := -L$(LIB_ROOT)/Custom -Wl,--gc-sections
else
    # Debug mode with symbol information
    CXXFLAGS := $(BASE_FLAGS) -O0 -g3
    LDFLAGS  := -L$(LIB_ROOT)/Custom
endif

CPPFLAGS := -I$(LIB_ROOT)
LIBS     := -lCGLib -lX11 -lX11-xcb -lxcb -lxcb-cursor

# --- File Discovery ---
SRCS := $(wildcard $(SRC_DIR)/*.cpp)
EXES := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%,$(SRCS))
DEPS := $(patsubst $(SRC_DIR)/%.cpp,$(DEP_DIR)/%.d,$(SRCS))

# --- Rules ---

# Default target
all: build_external_libs $(EXES)

# Target for speed-optimized release build
release:
	@$(MAKE) BUILD_MODE=release all

# Target for size-optimized release build
srelease:
	@$(MAKE) BUILD_MODE=srelease all

# Create necessary build directories
$(BUILD_DIR) $(DEP_DIR):
	mkdir -p $@

# Build external library by calling its own Makefile
.PHONY: build_external_libs
build_external_libs:
	$(MAKE) -C $(LIB_ROOT)

# Executable build rule
$(BUILD_DIR)/%: $(SRC_DIR)/%.cpp $(CGLIB_PATH) | $(BUILD_DIR) $(DEP_DIR)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -MF $(DEP_DIR)/$*.d $< -o $@ $(LDFLAGS) $(LIBS)
ifneq ($(BUILD_MODE), debug)
	@echo "Stripping symbols for $(BUILD_MODE) build: $@"
	@$(STRIP) --strip-all $@
endif

# Include generated dependency files (.d)
-include $(DEPS)

# Remove all build artifacts
clean:
	rm -rf bin/

.PHONY: all clean build_external_libs release srelease

