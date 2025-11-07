cmake_minimum_required(VERSION 3.10)

# here we specify that the project is C language only, so the default
# C compiler on the system will be used
project(cozenage C)
set(CMAKE_C_STANDARD 23)
#set(CMAKE_C_STANDARD_REQUIRED ON)
#set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_C_FLAGS "-Wall -Wextra -O0 -g") # -g -fsanitize=address,undefined")

## Option to force libedit
#option(USE_LIBEDIT "Force using libedit instead of GNU readline" OFF)
#
#if (USE_LIBEDIT)
#    find_library(READLINE_LIBRARY edit)
#    find_path(READLINE_INCLUDE_DIR editline/readline.h)
#else()
#    find_library(READLINE_LIBRARY readline)
#    find_path(READLINE_INCLUDE_DIR readline/readline.h)
#
#    # fallback to libedit if readline not found
#    if (NOT READLINE_LIBRARY)
#        find_library(READLINE_LIBRARY edit)
#        find_path(READLINE_INCLUDE_DIR editline/readline.h)
#    endif()
#endif()
#
## If neither found, error out
#if (NOT READLINE_LIBRARY OR NOT READLINE_INCLUDE_DIR)
#    message(FATAL_ERROR "Neither GNU readline nor libedit were found")
#endif()
#
## Print what we found
#message(STATUS "READLINE_LIBRARY=${READLINE_LIBRARY}")
#get_filename_component(LIBFILE ${READLINE_LIBRARY} NAME)
#if (LIBFILE MATCHES "readline")
#    message(STATUS "Using GNU readline")
#elseif (LIBFILE MATCHES "edit")
#    message(STATUS "Using libedit")
#endif()
#
#include_directories(${READLINE_INCLUDE_DIR})
#



# ==============================================================================
# Find Readline or Libedit
#
# This logic is designed to be robust, especially on macOS where system stub
# libraries can be misleading.
#
# 1. An option 'USE_LIBEDIT' allows the user to force the use of libedit.
# 2. If not forced, it prefers GNU readline found via pkg-config (common for Homebrew).
# 3. If pkg-config fails to find readline, it gracefully falls back to finding libedit.
# 4. It provides clear and accurate status messages about which library was found.
# ==============================================================================

option(USE_LIBEDIT "Force using libedit instead of GNU readline" OFF)

# We need the PkgConfig module to find libraries installed by package managers.
find_package(PkgConfig QUIET)

# This variable will hold the name of the library provider for accurate reporting.
set(READLINE_PROVIDER "None")

# --- Primary Logic ---

if (USE_LIBEDIT)
    message(STATUS "USE_LIBEDIT is ON. Forcing search for libedit.")
    set(SEARCH_FOR_LIBEDIT TRUE)

else()
    # Default behavior: Prefer GNU readline via pkg-config.
    if (PKG_CONFIG_FOUND)
        # We use a prefix "PC_READLINE" to keep variables from pkg-config local.
        pkg_check_modules(PC_READLINE QUIET readline)
    endif()

    if (PC_READLINE_FOUND)
        message(STATUS "Found GNU readline via pkg-config.")
        set(READLINE_PROVIDER "GNU readline (via pkg-config)")
        set(READLINE_LIBRARIES ${PC_READLINE_LIBRARIES})
        set(READLINE_INCLUDE_DIRS ${PC_READLINE_INCLUDE_DIRS})
        set(READLINE_DEFINITIONS ${PC_READLINE_CFLAGS_OTHER})
    else()
        # Fallback condition: pkg-config did not find GNU readline.
        message(STATUS "pkg-config could not find GNU readline. Falling back to libedit.")
        set(SEARCH_FOR_LIBEDIT TRUE)
    endif()
endif()

# --- Fallback Search ---
# This block only runs if USE_LIBEDIT was ON or if pkg-config failed.
if (SEARCH_FOR_LIBEDIT)
    find_library(LIBEDIT_LIBRARY NAMES edit)
    find_path(LIBEDIT_INCLUDE_DIR NAMES editline/readline.h)

    if (LIBEDIT_LIBRARY AND LIBEDIT_INCLUDE_DIR)
        set(READLINE_PROVIDER "libedit")
        set(READLINE_LIBRARIES ${LIBEDIT_LIBRARY})
        set(READLINE_INCLUDE_DIRS ${LIBEDIT_INCLUDE_DIR})
        # No special compile definitions for libedit
        set(READLINE_DEFINITIONS "")
    endif()
endif()


# --- Final Validation and Integration ---

if (READLINE_PROVIDER STREQUAL "None")
    message(FATAL_ERROR "Neither GNU readline nor libedit could be found. Please install one (e.g., 'brew install readline') or check your PKG_CONFIG_PATH.")
else()
    # Add the necessary include directories for your target.
    include_directories(${READLINE_INCLUDE_DIRS})

    # Add any compile definitions (e.g., -DHAVE_READLINE) if needed.
    add_definitions(${READLINE_DEFINITIONS})

    # Print a final, accurate summary.
    message(STATUS "Using ${READLINE_PROVIDER} for line editing.")
    message(STATUS "  - Includes: ${READLINE_INCLUDE_DIRS}")
    message(STATUS "  - Library:  ${READLINE_LIBRARIES}")

endif()


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

# Define your project executable
add_executable(cozenage src/main.c) # Start with just main

# Use a glob to find all your source files recursively
file(GLOB_RECURSE COZENAGE_SOURCES "src/*.c")

# Add these sources to your target
target_sources(cozenage PRIVATE ${COZENAGE_SOURCES})

# IMPORTANT: Tell CMake where to find header files!
target_include_directories(cozenage PRIVATE
        ${PROJECT_SOURCE_DIR}/src
        ${PROJECT_SOURCE_DIR}/src/scheme-lib
        ${PROJECT_SOURCE_DIR}/src/cozenage-lib
)

target_link_libraries(cozenage PRIVATE ${READLINE_LIBRARIES} ${M_LIB} ${GC_LIB} icu::uc)

# If we found GNU readline, pass a definition to the C code.
if (READLINE_PROVIDER MATCHES "GNU readline")
    message(STATUS "Setting compile definition: USE_GNU_READLINE")
    target_compile_definitions(cozenage PRIVATE USE_GNU_READLINE)
endif()
