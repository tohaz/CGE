# --- Compiler Settings ---
CXX = g++
CXXFLAGS = -Wall -Wextra -g -std=c++20
INCLUDE_PATH = ../
CPPFLAGS = -I$(INCLUDE_PATH)
LDFLAGS = -Llib -L../Custom/
LIBS = -lCGLib -lX11 -lX11-xcb -lxcb -lxcb-cursor

# --- Paths ---
# Path to the static library file (dependency)
CGLIB_PATH = ../Custom/libCGLib.a

SRC_DIR = ./
BUILD_DIR = ./bin/

# --- File Discovery ---
SRCS = $(wildcard $(SRC_DIR)*.cpp)
EXES = $(patsubst $(SRC_DIR)%.cpp,$(BUILD_DIR)%,$(SRCS))

# --- Special Targets ---

# Automatically delete the target file if the recipe fails during execution
.DELETE_ON_ERROR:

all: $(EXES)

# Create the build directory if it doesn't exist
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Target to trigger build in the parent directory
.PHONY: build_external_libs
build_external_libs:
	$(MAKE) -C ..

# Link the library path to the external build command
$(CGLIB_PATH): build_external_libs
	@:

# --- Build Rule for Executables ---
# 'rm -f $@' is executed FIRST. This ensures that if the compiler fails 
# later, the old binary is already gone and won't persist.
$(BUILD_DIR)%: $(SRC_DIR)%.cpp $(CGLIB_PATH) | $(BUILD_DIR)
	rm -f $@
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $< -o $@ $(LDFLAGS) $(LIBS)

# --- Cleanup ---
.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
