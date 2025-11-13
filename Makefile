#
# Unified Makefile for the 'cozenage' project
#
# Targets:
#   make / make all      - Builds the project using CMake (default).
#   make nocmake         - Builds the project manually without CMake.
#   make test            - Builds the test runner.
#   make clean           - Removes all built artifacts.
#   make rebuild         - Cleans and rebuilds using the default (CMake) method.
#

# --- Primary Variables ---
CC = gcc
BINARY = cozenage
TEST_BINARY = run_tests
BUILD_DIR = build
OBJ_DIR = obj

# MODIFICATION: Added OS detection for library extensions
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    LIB_EXT = dylib
    # Flag for modules: Allow undefined symbols (will be found in main binary)
	MODULE_LDFLAGS = -Wl,-undefined,dynamic_lookup
	# Flag for main executable: None needed on macOS
	EXE_LDFLAGS =
else
    LIB_EXT = so
    # Flag for modules: None needed on Linux
	MODULE_LDFLAGS =
	# Flag for main executable: Export symbols for modules to find
	EXE_LDFLAGS = -Wl,--export-dynamic
endif

# MODIFICATION: Added flags for building shared libraries
LIB_CFLAGS = -shared -fPIC

# --- Source and Object Files ---

# MODIFICATION: Separated Core sources from Lib sources
CORE_SOURCE_DIRS = src
LIB_SOURCE_DIRS  = src/base-lib
TEST_SOURCE_DIRS = tests
ALL_SOURCE_DIRS  = $(CORE_SOURCE_DIRS) $(LIB_SOURCE_DIRS) $(TEST_SOURCE_DIRS)

# MODIFICATION: Discover sources from their specific directories
CORE_SOURCES = $(foreach dir,$(CORE_SOURCE_DIRS),$(wildcard $(dir)/*.c))
LIB_SOURCES  = $(foreach dir,$(LIB_SOURCE_DIRS),$(wildcard $(dir)/*.c))
TEST_SOURCES = $(foreach dir,$(TEST_SOURCE_DIRS),$(wildcard $(dir)/*.c))

# MODIFICATION: Objects for main binary now *only* come from CORE_SOURCES
CORE_OBJECTS = $(patsubst %.c,$(OBJ_DIR)/%.o,$(CORE_SOURCES))

# MODIFICATION: New variable for all the loadable module files
# e.g., lib/math.so, lib/file.so
LIB_MODULES  = $(patsubst src/base-lib/%_lib.c,lib/%.$(LIB_EXT),$(LIB_SOURCES))

# MODIFICATION: Test sources now correctly use CORE_SOURCES
APP_SOURCES_FOR_TEST = $(filter-out src/main.c, $(CORE_SOURCES))
ALL_SOURCES_FOR_TEST = $(APP_SOURCES_FOR_TEST) $(TEST_SOURCES)
TEST_OBJECTS = $(patsubst %.c,$(OBJ_DIR)/%.o,$(ALL_SOURCES_FOR_TEST))

# --- Compiler Flags ---
# Add include paths for ALL source directories to CFLAGS
CFLAGS = $(foreach dir,$(ALL_SOURCE_DIRS),-I$(dir))

# Detect ICU flags using pkg-config
ICU_CFLAGS = $(shell pkg-config --cflags icu-uc)
ICU_LIBS = $(shell pkg-config --libs icu-uc)

# Specific flag sets for different builds
CFLAGS_DEFAULT = -Wall -Wextra -Werror -Wdeprecated-declarations -O2 -std=gnu2x $(ICU_CFLAGS)
CFLAGS_TEST = -Wall -Wextra -g -O0 -std=gnu2x $(ICU_CFLAGS) -fsanitize=address -fno-omit-frame-pointer

# --- Libraries ---
# Auto-detect readline or libedit
# MODIFICATION: Added -ldl (for dlopen) to all BASE_LIBS definitions
ifeq ($(shell pkg-config --exists readline && echo yes),yes)
    BASE_LIBS = -lreadline -lm -lgc $(ICU_LIBS) -ldl $(EXE_LDFLAGS)
else ifeq ($(shell pkg-config --exists edit && echo yes),yes)
    BASE_LIBS = -ledit -lm -lgc $(ICU_LIBS) -ldl $(EXE_LDFLAGS)
else
    BASE_LIBS = -lreadline -lm -lgc $(ICU_LIBS) -ldl $(EXE_LDFLAGS)
endif
TEST_LIBS = -lcriterion $(BASE_LIBS)

# --- Phony Targets (Commands) ---
.PHONY: all cmake_build nocmake test clean rebuild

# The default target when 'make' is run
all: cmake_build

# Target to build using CMake (unchanged)
cmake_build:
	@echo "--- Building with CMake ---"
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake ..
	@$(MAKE) -C $(BUILD_DIR)
	@cp $(BUILD_DIR)/$(BINARY) .

# Target to build manually (without CMake)
nocmake: CFLAGS += $(CFLAGS_DEFAULT)
# MODIFICATION: 'nocmake' now also depends on building all the modules
nocmake: $(BINARY) $(LIB_MODULES)
	@echo "--- Manual build complete: ./$(BINARY) and modules in lib/ ---"

# Target to build the test runner
test: CFLAGS += $(CFLAGS_TEST)
test: $(TEST_BINARY)
	@echo "--- Test build complete: ./$(TEST_BINARY) ---"

# Target to clean all artifacts from all build methods
clean:
	@echo "--- Cleaning all build artifacts ---"
	@rm -f $(BINARY) $(TEST_BINARY)
	# MODIFICATION: Also remove the 'lib' directory
	@rm -rf $(BUILD_DIR) $(OBJ_DIR) lib

# Target to clean and then rebuild using the default method
rebuild: clean all

# --- File-Generating Rules ---

# MODIFICATION: Rule to link the main application now *only* uses CORE_OBJECTS
$(BINARY): $(CORE_OBJECTS)
	@echo "Linking application: $@"
	$(CC) $(CFLAGS) -o $@ $^ $(BASE_LIBS)

# Rule to link the test runner for 'test' build
$(TEST_BINARY): $(TEST_OBJECTS)
	@echo "Linking test runner: $@"
	$(CC) $(CFLAGS) -o $@ $^ $(TEST_LIBS) -fsanitize=address

# A static pattern rule to compile any .c file into its corresponding
# location inside the obj directory.
$(OBJ_DIR)/%.o: %.c
    # Ensure the target directory exists before compiling (e.g., obj/src/scheme-lib/)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

# $< is the first prerequisite (the .c file)
# $@ is the target (the .o file)
# $(@D) is the directory part of the target


# ==============================================================================
# MODIFICATION: New Rule to Build Loadable Modules
#
# This rule matches, for example, 'lib/math.so' with
# 'src/base-lib/math_lib.c' and compiles it as a shared library.
# ==============================================================================
lib/%.$(LIB_EXT): src/base-lib/%_lib.c
	@mkdir -p lib
	@echo "Building module: $@"
	$(CC) $(CFLAGS) $(LIB_CFLAGS) $(MODULE_LDFLAGS) $< -o $@

