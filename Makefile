#
# Unified Makefile for the 'cozenage' project
#
# Targets:
#   make / make all      - Builds the cozenage binary and loadable modules.
#   make DEBUG=1         - builds unoptimized binary and modules with debug symbols.
#   make test            - Builds the test runner.
#   make clean           - Removes all build artifacts
#   make rebuild         - Cleans and rebuilds the main binary and modules
#   make install         - installs the binary to ${PREFIX}/bin/cozenage
#                           and the modules to $(PREFIX)/lib/cozenage/
#                           Override default using: $ make install PREFIX=/my/custom/path
#   make uninstall       - deletes the binary and module directory.

# --- Primary Variables ---
CC ?= cc
BINARY = cozenage
TEST_BINARY = run_tests
OBJ_DIR = obj
PROD_OBJ_DIR = $(OBJ_DIR)/prod
TEST_OBJ_DIR = $(OBJ_DIR)/test
# Install targets - prefix configurable via:
# `make install PREFIX=/path/to/install`
PREFIX ?= /usr/local
DESTDIR ?=
INSTALL_BIN_DIR=$(DESTDIR)$(PREFIX)/bin
INSTALL_LIB_DIR=$(DESTDIR)$(PREFIX)/lib/cozenage/base

# OS detection for library extensions
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	LIB_EXT = dylib
	MODULE_LDFLAGS = -Wl,-undefined,dynamic_lookup
	# Add RPATH to the executable
	EXE_LDFLAGS = -Wl,-rpath,@executable_path/../lib/cozenage/base/
else
	LIB_EXT = so
	MODULE_LDFLAGS =
	# Export symbols AND add RPATH using $ORIGIN
	# Note: use \$$ to ensure the '$' reaches the shell/linker correctly
	EXE_LDFLAGS = -Wl,--export-dynamic -Wl,-rpath,'\$$ORIGIN/../lib/cozenage/base'
endif

# Check for debug build flag
ifeq ($(origin DEBUG), undefined)
	DEBUG=OFF
endif

# Flags for building shared libraries
LIB_CFLAGS = -shared -fPIC

# --- Source and Object Files ---

# Separated Core sources from Lib sources
CORE_SOURCE_DIRS = src
LIB_SOURCE_DIRS  = src/base-lib
TEST_SOURCE_DIRS = tests
ALL_SOURCE_DIRS  = $(CORE_SOURCE_DIRS) $(LIB_SOURCE_DIRS) $(TEST_SOURCE_DIRS)

# Discover sources from their specific directories
CORE_SOURCES = $(foreach dir,$(CORE_SOURCE_DIRS),$(wildcard $(dir)/*.c))
LIB_SOURCES := $(foreach dir,$(LIB_SOURCE_DIRS),$(wildcard $(dir)/*.c))
TEST_SOURCES = $(foreach dir,$(TEST_SOURCE_DIRS),$(wildcard $(dir)/*.c))

# Objects for main binary now *only* come from CORE_SOURCES
CORE_OBJECTS = $(patsubst %.c,$(PROD_OBJ_DIR)/%.o,$(CORE_SOURCES))

# Test sources use CORE_SOURCES
APP_SOURCES_FOR_TEST = $(filter-out src/main.c, $(CORE_SOURCES))
ALL_SOURCES_FOR_TEST = $(APP_SOURCES_FOR_TEST) $(TEST_SOURCES)
TEST_OBJECTS = $(patsubst %.c,$(TEST_OBJ_DIR)/%.o,$(ALL_SOURCES_FOR_TEST))

# --- Compiler Flags ---

# Use := to execute exactly once at parse time
ICU_VERSION := $(shell pkg-config --modversion icu-uc 2>/dev/null)
GMP_VERSION := $(shell pkg-config --modversion gmp 2>/dev/null)
GC_VERSION  := $(shell pkg-config --modversion bdw-gc 2>/dev/null)

# Abort immediately if any are empty
ifeq ($(ICU_VERSION),)
  $(error "Hard dependency 'icu-uc' not found. Please install ICU.")
endif

ifeq ($(GMP_VERSION),)
  $(error "Hard dependency 'gmp' not found. Please install GMP.")
endif

ifeq ($(GC_VERSION),)
  $(error "Hard dependency 'bdw-gc' not found. Please install libgc.")
endif

# Detect ICU flags using pkg-config
ICU_CFLAGS = $(shell pkg-config --cflags icu-uc)
ICU_LIBS = $(shell pkg-config --libs icu-uc)

# Detect gmp flags and libs
GMP_CFLAGS = $(shell pkg-config --cflags gmp)
GMP_LIBS = $(shell pkg-config --libs gmp)

# Detect libgc flags and libs
GC_CFLAGS = $(shell pkg-config --cflags bdw-gc)
GC_LIBS = $(shell pkg-config --libs bdw-gc)

# Mandatory flags (includes, language standard, pkg-config)
APP_CFLAGS = -std=gnu2x \
             $(foreach dir,$(ALL_SOURCE_DIRS),-I$(dir)) \
             $(ICU_CFLAGS) $(GMP_CFLAGS) $(GC_CFLAGS)

# Detect criterion flags and libs
# Check if 'test' is anywhere in the command line args (e.g., 'make test')
ifneq ($(filter test,$(MAKECMDGOALS)),)
  CRITERION_VERSION := $(shell pkg-config --modversion criterion 2>/dev/null)
  
  ifeq ($(CRITERION_VERSION),)
    $(error "Hard dependency 'criterion' not found. Required to run 'make test'.")
  endif
  
  CRIT_CFLAGS = $(shell pkg-config --cflags criterion)
  CRIT_LIBS   = $(shell pkg-config --libs criterion)
endif

SSL_VERSION = $(shell pkg-config --modversion openssl || echo "Not found! 'random' module will not be built")

# Detect openssl lib, and omit random.so compilation if not present
LIB_MODULES := $(patsubst src/base-lib/%_lib.c,lib/cozenage/base/%.$(LIB_EXT),$(LIB_SOURCES))

ifeq ($(shell pkg-config --exists openssl && echo yes),yes)
	SSL_CFLAGS = $(shell pkg-config --cflags openssl)
	SSL_LIBS := $(shell pkg-config --libs openssl)
	MODULE_LDFLAGS += $(SSL_LIBS)
	APP_CFLAGS += $(SSL_CFLAGS)
else
	LIB_MODULES := $(filter-out lib/cozenage/base/random.$(LIB_EXT),$(LIB_MODULES))
endif

# Specific flag sets for different builds
CFLAGS ?= -Wall -Wextra -Wdeprecated-declarations -O2

# --- Libraries ---
# -ldl (for dlopen) to all BASE_LIBS definitions
BASE_LIBS = -lm $(GC_LIBS) $(ICU_LIBS) -ldl $(EXE_LDFLAGS) $(GMP_LIBS)
TEST_LIBS = $(CRIT_LIBS) $(BASE_LIBS)

# --- Phony Targets (Commands) ---
.PHONY: all test clean rebuild install uninstall docs docs-clean

# The default target when 'make' is run
all:
	@$(MAKE) print_msg
	@$(MAKE) cozenage_build

# Target to build main binary and modules
cozenage_build: $(BINARY) $(LIB_MODULES)
	@echo "\x1b[32;1m---- Manual build complete: ./$(BINARY) and modules in lib/cozenage/base/ ---\x1b[0m"

# Target to build the test runner
test: APP_CFLAGS += -g -O0 $(CRIT_CFLAGS) -DCRITERION_TEST_BUILD
test: $(TEST_BINARY)
	@echo "\x1b[32;1m--- Test build complete: ./$(TEST_BINARY) ---\x1b[0m"

print_msg:
	@echo "\x1b[32;1m--- Building cozenage binary ---\x1b[0m"
	@echo "    ICU version       $(ICU_VERSION)"
	@echo "    GMP version       $(GMP_VERSION)"
	@echo "    libgc version     $(GC_VERSION)"
	@echo "    OpenSSL version   $(SSL_VERSION)"
	@echo "-------------------------------------"


# Target to clean all artifacts from all build directories
clean:
	@echo "\x1b[32;1m--- Cleaning all build artifacts ---\x1b[0m"
	@rm -f $(BINARY) $(TEST_BINARY)
	@rm -rf $(OBJ_DIR) lib/cozenage

# Target to clean and then rebuild
rebuild: clean all

# --- File-Generating Rules ---

# $< is the first prerequisite (the .c file)
# $@ is the target (the .o file)
# $(@D) is the directory part of the target

# Rule for production objects
$(PROD_OBJ_DIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(APP_CFLAGS) $(CFLAGS) -c $< -o $@

# Rule for test objects
$(TEST_OBJ_DIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(APP_CFLAGS) -c $< -o $@

# Apply the same logic to your $(BINARY), $(TEST_BINARY), and loadable module rules
$(BINARY): $(CORE_OBJECTS)
	@echo "\x1b[32;1m--- Linking application: $@ ---\x1b[0m"
	$(CC) $(APP_CFLAGS) $(CFLAGS) -o $@ $^ $(BASE_LIBS)

# Rule to link the test runner for 'test' build
$(TEST_BINARY): $(TEST_OBJECTS)
	@echo "Linking test runner: $@"
	$(CC) $(APP_CFLAGS) -o $@ $^ $(TEST_LIBS) -fsanitize=address

# Rule to build modules
lib/cozenage/base/%.$(LIB_EXT): src/base-lib/%_lib.c
	@mkdir -p $(@D)
	@echo "\x1b[32;1m--- Building module: $@ ---\x1b[0m"
	$(CC) $(APP_CFLAGS) $(LIB_CFLAGS) $(MODULE_LDFLAGS) $(CFLAGS) $< -o $@

# --- install rules
install:
	@if [ ! -f $(BINARY) ]; then \
		echo "ERROR: $(BINARY) not found. Please run 'make' before 'make install'."; \
		exit 1; \
	fi
	@mkdir -v -p $(INSTALL_BIN_DIR)
	@mkdir -v -p $(INSTALL_LIB_DIR)
	@install -v -m 755 $(BINARY) $(INSTALL_BIN_DIR)
	@for m in lib/cozenage/base/*.$(LIB_EXT); do \
    	install -v -m 755 $$m $(INSTALL_LIB_DIR)/$$(basename $$m); \
    done
	@echo "-- Installed $(BINARY) to $(INSTALL_BIN_DIR)"
	@echo "-- Installed modules to $(INSTALL_LIB_DIR)"

uninstall:
	@rm -v -f $(INSTALL_BIN_DIR)/$(BINARY)
	@rm -v -rf $(INSTALL_LIB_DIR)

# --- Docs
docs:
	@$(MAKE) -C docs/source html

docs-clean:
	@$(MAKE) -C docs/source clean
