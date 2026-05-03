# --- Paths (Relative) ---
LIB_ROOT   := ..
CGLIB_PATH := $(LIB_ROOT)/Custom/libCGLib.a

SRC_DIR    := .
BUILD_DIR  := bin
# Path for dependency files inside bin
DEP_DIR    := $(BUILD_DIR)/deps

# --- Compiler Settings ---
CXX := g++
# We use -MF to explicitly tell GCC where to put the .d file
CXXFLAGS := -Wall -Wextra -g -std=c++26 -O0 -g3 -fmessage-length=0 -MMD -MP
CPPFLAGS := -I$(LIB_ROOT)
LDFLAGS  := -L$(LIB_ROOT)/Custom
LIBS     := -lCGLib -lX11 -lX11-xcb -lxcb -lxcb-cursor

# --- File Discovery ---
SRCS := $(wildcard $(SRC_DIR)/*.cpp)
EXES := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%,$(SRCS))
DEPS := $(patsubst $(SRC_DIR)/%.cpp,$(DEP_DIR)/%.d,$(SRCS))

# --- Rules ---

all: build_external_libs $(EXES)

# Create both bin and bin/deps directories
$(BUILD_DIR) $(DEP_DIR):
	mkdir -p $@

# Call the parent makefile
.PHONY: build_external_libs
build_external_libs:
	$(MAKE) -C $(LIB_ROOT)

# Rule for building executables
# We add -MF $(DEP_DIR)/$*.d to redirect the dependency file
$(BUILD_DIR)/%: $(SRC_DIR)/%.cpp $(CGLIB_PATH) | $(BUILD_DIR) $(DEP_DIR)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -MF $(DEP_DIR)/$*.d $< -o $@ $(LDFLAGS) $(LIBS)

# Include generated dependencies from the new subdirectory
-include $(DEPS)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean build_external_libs

