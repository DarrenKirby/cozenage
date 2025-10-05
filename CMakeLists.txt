cmake_minimum_required(VERSION 3.10)

# here we specify that the project is C language only, so the default
# C compiler on the system will be used
project(cozenage C)
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_FLAGS "-Wall -Wextra -O2") # -g -fsanitize=address,undefined")

# Option to force libedit
option(USE_LIBEDIT "Force using libedit instead of GNU readline" OFF)

if (USE_LIBEDIT)
    find_library(READLINE_LIBRARY edit)
    find_path(READLINE_INCLUDE_DIR editline/readline.h)
else()
    find_library(READLINE_LIBRARY readline)
    find_path(READLINE_INCLUDE_DIR readline/readline.h)

    # fallback to libedit if readline not found
    if (NOT READLINE_LIBRARY)
        find_library(READLINE_LIBRARY edit)
        find_path(READLINE_INCLUDE_DIR editline/readline.h)
    endif()
endif()

# If neither found, error out
if (NOT READLINE_LIBRARY OR NOT READLINE_INCLUDE_DIR)
    message(FATAL_ERROR "Neither GNU readline nor libedit were found")
endif()

# Print what we found
message(STATUS "READLINE_LIBRARY=${READLINE_LIBRARY}")
get_filename_component(LIBFILE ${READLINE_LIBRARY} NAME)
if (LIBFILE MATCHES "readline")
    message(STATUS "Using GNU readline")
elseif (LIBFILE MATCHES "edit")
    message(STATUS "Using libedit")
endif()

include_directories(${READLINE_INCLUDE_DIR})

# --- Find ICU Library

# 1. First, try CMake's standard "find module". This works for standard installs.
#    We request the 'uc' component specifically.
find_package(ICU COMPONENTS uc QUIET)

# 2. If the find module fails, then we try to fall back to pkg-config.
if(NOT ICU_FOUND)
    message(STATUS "CMake's FindICU module failed. Falling back to pkg-config...")
    find_package(PkgConfig QUIET)

    if(PKG_CONFIG_FOUND)
        pkg_check_modules(PC_ICU QUIET icu-uc)
    endif()
endif()

# 3. Create a single, unified 'icu::uc' alias target so we can link to it
#    in the same way regardless of how it was found.
if(ICU_FOUND OR PC_ICU_FOUND)
    if(NOT TARGET icu::uc)
        add_library(icu::uc INTERFACE IMPORTED)
    endif()

    if(ICU_FOUND)
        # The library was found by the standard find_package(ICU) command.
        # Its target is ICU::uc, so we link our alias to it.
        message(STATUS "Found ICU via standard find_package.")
        target_link_libraries(icu::uc INTERFACE ICU::uc)
    else()
        # The library was found using pkg-config. We configure our alias
        # with the properties discovered by pkg_check_modules.
        message(STATUS "Found ICU via pkg-config.")
        target_include_directories(icu::uc INTERFACE ${PC_ICU_INCLUDE_DIRS})
        target_link_directories(icu::uc INTERFACE ${PC_ICU_LIBRARY_DIRS}) # <-- THIS IS THE FIX
        target_link_libraries(icu::uc INTERFACE ${PC_ICU_LIBRARIES})
    endif()
else()
    # 4. If both methods fail, stop the build with a helpful error message.
    message(FATAL_ERROR "Could not find the ICU library.\n"
            "Please ensure ICU is installed.\n"
            "- For standard installs, check your system PATH.\n"
            "- For Homebrew/custom installs, ensure 'pkg-config' is installed and can find 'icu-uc'.")
endif()

# Find math/gc library
find_library(M_LIB m)
find_library(GC_LIB gc)

add_executable(cozenage src/bools.c
        src/bytevectors.c
        src/char_lib.c
        src/chars.c
        src/comparators.c
        src/complex_lib.c
        src/control_features.c
        src/coz_ext_lib.c
        src/environment.c
        src/eval.c
        src/eval_lib.c
        src/file_lib.c
        src/inexact_lib.c
        src/io_lib.c
        src/load_library.c
        src/main.c
        src/numerics.c
        src/pairs.c
        src/parser.c
        src/ports.c
        src/predicates.c
        src/printer.c
        src/process_context_lib.c
        src/special_forms.c
        src/strings.c
        src/types.c
        src/vectors.c
        src/cxr_lib.c
        src/cxr_lib.h
        src/coz_bits_lib.c
        src/coz_bits_lib.h)

target_link_libraries(cozenage PRIVATE ${READLINE_LIBRARY} ${M_LIB} ${GC_LIB} icu::uc)
