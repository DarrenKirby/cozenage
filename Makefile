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

# --- Source and Object Files ---
# VPATH tells make where to find source files
VPATH = src:tests

# Application sources and objects
APP_SOURCES = $(wildcard src/*.c)
APP_OBJECTS = $(patsubst src/%.c, %.o, $(APP_SOURCES))

# Test sources and objects (excludes main.c)
APP_SOURCES_FOR_TEST = $(filter-out src/main.c, $(APP_SOURCES))
TEST_SOURCES = $(wildcard tests/*.c)
TEST_OBJECTS = $(patsubst src/%.c, %.o, $(APP_SOURCES_FOR_TEST)) \
               $(patsubst tests/%.c, %.o, $(TEST_SOURCES))

# Detect ICU flags using pkg-config
ICU_CFLAGS = $(shell pkg-config --cflags icu-uc)
ICU_LIBS = $(shell pkg-config --libs icu-uc)

# --- Compiler Flags ---
# CFLAGS are set per-target later on
CFLAGS_DEFAULT = -Wall -Wextra -O2 -std=gnu2x $(ICU_CFLAGS)
CFLAGS_TEST = -fsanitize=address,undefined -Wall -Wextra -g -O0 -DTESTING__ $(ICU_CFLAGS)

# --- Libraries ---
# Auto-detect readline or libedit for manual and test builds
ifeq ($(shell pkg-config --exists readline && echo yes),yes)
    BASE_LIBS = -lreadline -lm -lgc $(ICU_LIBS)
else ifeq ($(shell pkg-config --exists edit && echo yes),yes)
    BASE_LIBS = -ledit -lm -lgc $(ICU_LIBS)
else
    BASE_LIBS = -lreadline -lm -lgc $(ICU_LIBS)
endif

# Test-specific libraries
TEST_LIBS = -lcriterion $(BASE_LIBS)

# --- Phony Targets (Commands) ---
.PHONY: all cmake_build nocmake test clean rebuild

# The default target when 'make' is run
all: cmake_build

# Target to build using CMake
cmake_build:
	@echo "--- Building with CMake ---"
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake ..
	@$(MAKE) -C $(BUILD_DIR)
	@cp $(BUILD_DIR)/$(BINARY) .

# Target to build manually (without CMake)
# We set CFLAGS specifically for this target and its prerequisites
nocmake: CFLAGS = $(CFLAGS_DEFAULT)
nocmake: $(BINARY)
	@echo "--- Manual build complete: ./$(BINARY) ---"

# Target to build the test runner
# We set CFLAGS specifically for this target and its prerequisites
test: CFLAGS = $(CFLAGS_TEST)
test: $(TEST_BINARY)
	@echo "--- Test build complete: ./$(TEST_BINARY) ---"

# Target to clean all artifacts from all build methods
clean:
	@echo "--- Cleaning all build artifacts ---"
	@rm -f $(BINARY) $(TEST_BINARY) *.o
	@rm -rf $(BUILD_DIR)

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

# Generic rule to compile any .c file into a .o file
# It finds sources in VPATH (src/ or tests/) and inherits CFLAGS from the
# target that triggered the build (nocmake or test).
%.o: %.c
#	@echo "Compiling: $<"
	$(CC) $(CFLAGS) -Isrc -c $< -o $@
