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

# --- Source and Object Files ---

# Define all directories containing C source files
APP_SOURCE_DIRS  = src src/scheme-lib src/cozenage-lib
TEST_SOURCE_DIRS = tests
ALL_SOURCE_DIRS  = $(APP_SOURCE_DIRS) $(TEST_SOURCE_DIRS)

# Discover all source files from their respective directories
APP_SOURCES  = $(foreach dir,$(APP_SOURCE_DIRS),$(wildcard $(dir)/*.c))
TEST_SOURCES = $(foreach dir,$(TEST_SOURCE_DIRS),$(wildcard $(dir)/*.c))

# Create flattened lists of object file paths inside OBJ_DIR
APP_OBJECTS  = $(addprefix $(OBJ_DIR)/, $(notdir $(patsubst %.c,%.o,$(APP_SOURCES))))

# For the test build, we use all app sources EXCEPT main.c, plus all test sources
APP_SOURCES_FOR_TEST = $(filter-out src/main.c, $(APP_SOURCES))
ALL_SOURCES_FOR_TEST = $(APP_SOURCES_FOR_TEST) $(TEST_SOURCES)
TEST_OBJECTS = $(addprefix $(OBJ_DIR)/, $(notdir $(patsubst %.c,%.o,$(ALL_SOURCES_FOR_TEST))))

# --- Compiler Flags ---
# Add include paths for ALL source directories to CFLAGS
CFLAGS = $(foreach dir,$(ALL_SOURCE_DIRS),-I$(dir))

# Detect ICU flags using pkg-config
ICU_CFLAGS = $(shell pkg-config --cflags icu-uc)
ICU_LIBS = $(shell pkg-config --libs icu-uc)

# Specific flag sets for different builds
CFLAGS_DEFAULT = -Wall -Wextra -O2 -std=gnu2x $(ICU_CFLAGS)
CFLAGS_TEST = -Wall -Wextra -g -O0 -std=gnu2x -DTESTING__ $(ICU_CFLAGS)

# --- Libraries ---
# Auto-detect readline or libedit
ifeq ($(shell pkg-config --exists readline && echo yes),yes)
    BASE_LIBS = -lreadline -lm -lgc $(ICU_LIBS)
else ifeq ($(shell pkg-config --exists edit && echo yes),yes)
    BASE_LIBS = -ledit -lm -lgc $(ICU_LIBS)
else
    BASE_LIBS = -lreadline -lm -lgc $(ICU_LIBS)
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
nocmake: $(BINARY)
	@echo "--- Manual build complete: ./$(BINARY) ---"

# Target to build the test runner
test: CFLAGS += $(CFLAGS_TEST)
test: $(TEST_BINARY)
	@echo "--- Test build complete: ./$(TEST_BINARY) ---"

# Target to clean all artifacts from all build methods
clean:
	@echo "--- Cleaning all build artifacts ---"
	@rm -f $(BINARY) $(TEST_BINARY)
	@rm -rf $(BUILD_DIR) $(OBJ_DIR)

# Target to clean and then rebuild using the default method
rebuild: clean all

# --- File-Generating Rules ---

# Rule to link the main application for 'nocmake' build
$(BINARY): $(APP_OBJECTS)
	@echo "Linking application: $@"
	$(CC) $(CFLAGS) -o $@ $^ $(BASE_LIBS)

# Rule to link the test runner for 'test' build
$(TEST_BINARY): $(TEST_OBJECTS)
	@echo "Linking test runner: $@"
	$(CC) $(CFLAGS) -o $@ $^ $(TEST_LIBS)

# A single pattern rule to compile any .c file into the obj dir.
# This rule is now generic enough to handle both app and test sources.
$(OBJ_DIR)/%.o:
	# Ensure the object directory exists before compiling
	@mkdir -p $(OBJ_DIR)
	# Find the full path to the source file (.c) based on the object file name (.o)
	$(eval SOURCE_FILE := $(shell find $(ALL_SOURCE_DIRS) -name $*.c -print -quit))
	@echo "Compiling: $(SOURCE_FILE)"
	$(CC) $(CFLAGS) -c $(SOURCE_FILE) -o $@
