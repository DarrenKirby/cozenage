cmake_minimum_required(VERSION 3.10)

# here we specify that the project is C language only, so the default
# C compiler on the system will be used
project(cozenage C)
set(CMAKE_C_STANDARD 23)

option(DEBUG_BUILD "Enable debug CFLAGS" OFF)

if (DEBUG_BUILD)
    set(CMAKE_C_FLAGS "-march=native -Wall -Wextra -Werror -O0 -g -frounding-math -fsigned-zeros")
    message(STATUS "Building with debugging symbols enabled.")
else()
    set(CMAKE_C_FLAGS "-march=native -Wall -Wextra -Werror -O2 -frounding-math -fsigned-zeros")
endif()

message(STATUS "Building with CFLAGS: ${CMAKE_C_FLAGS}")

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
    message(FATAL_ERROR "Neither GNU readline nor libedit could be found. Please install one
                        (e.g., 'brew install readline') or check your PKG_CONFIG_PATH.")
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
# No need to check - standard C library

find_library(GC_LIB gc)
if(NOT GC_LIB)
    message(FATAL_ERROR "Could not find the 'gc' library. This is required for garbage collection.")
endif()

find_package(OpenSSL)
if(OpenSSL_FOUND)
    message(STATUS "Found OpenSSL version: ${OPENSSL_VERSION}. 'random' module will be built.")
else()
    message(STATUS "OpenSSL not found. 'random' module will NOT be built.")
endif()

# Find the 'dl' library (for dlopen, dlsym, etc.)
find_library(DL_LIB dl)
if(NOT DL_LIB)
    message(FATAL_ERROR "Could not find the 'dl' library. This is required for dynamic loading.")
endif()

find_library(GMP_LIB gmp)
if(NOT GMP_LIB)
    message(FATAL_ERROR "Could not find the 'gmp' library. This is required for bigints/bigfloats.")
else()
    message(STATUS "Found GNU GMP library for bigints and bigfloats.")
endif()

# Define your project executable
add_executable(cozenage src/main.c) # Start with just main

# Changed GLOB_RECURSE to GLOB and "src/*.c"
# This now *only* finds source files directly in src/,
# and *excludes* src/base-lib/
file(GLOB COZENAGE_SOURCES "src/*.c")

# Add these sources to your target
#  Removed src/main.c from this list as it's
# already in add_executable. This prevents it from being listed twice.
list(REMOVE_ITEM COZENAGE_SOURCES "${PROJECT_SOURCE_DIR}/src/main.c")
target_sources(cozenage PRIVATE ${COZENAGE_SOURCES})

# IMPORTANT: Tell CMake where to find header files!
target_include_directories(cozenage PRIVATE
        ${PROJECT_SOURCE_DIR}/src
        ${PROJECT_SOURCE_DIR}/src/base-lib
)

target_link_directories(cozenage PRIVATE ${PC_READLINE_LIBRARY_DIRS})

# Added ${DL_LIB} to the list of linked libraries.
target_link_libraries(cozenage PRIVATE
        ${READLINE_LIBRARIES}
        ${M_LIB}
        ${GC_LIB}
        ${DL_LIB}
        ${GMP_LIB}
        icu::uc
)

# If we found GNU readline, pass a definition to the C code.
if (READLINE_PROVIDER MATCHES "GNU readline")
    message(STATUS "Setting compile definition: USE_GNU_READLINE")
    target_compile_definitions(cozenage PRIVATE USE_GNU_READLINE)
endif()


# ==============================================================================
# Build Loadable Modules
#
# This new section finds all .c files in src/base-lib/ and builds
# each one as a separate shared library, outputting them to the 'lib/'
# directory in the project's root.
# ==============================================================================

# Define the output directory for modules (e.g., ./lib)
set(MODULE_OUTPUT_DIR ${PROJECT_SOURCE_DIR}/lib)
file(MAKE_DIRECTORY ${MODULE_OUTPUT_DIR})

# Find all the .c files in the library directory
file(GLOB LIB_SOURCES "src/base-lib/*.c")

foreach(LIB_SOURCE ${LIB_SOURCES})
    # Get the filename without extension (e.g., "math_lib")
    get_filename_component(LIB_NAME ${LIB_SOURCE} NAME_WE)

    # Strip "_lib" to get the logical target name (e.g., "math")
    string(REPLACE "_lib" "" LIB_TARGET_NAME ${LIB_NAME})

    # --- Check for Optional Dependency (OpenSSL) ---
    if(LIB_TARGET_NAME STREQUAL "random" AND NOT OpenSSL_FOUND)
        # If OpenSSL is missing, we skip all build commands for this file.
        # CMake will simply start the next iteration of the loop.
        continue() # Use 'continue()' for clarity (requires CMake 3.1)
    endif()

    # Add the library as a SHARED module
    add_library(${LIB_TARGET_NAME} SHARED ${LIB_SOURCE})

    # Set properties:
    # 1. PREFIX: Set to "" to get "math.so" instead of "libmath.so"
    # 2. LIBRARY_OUTPUT_DIRECTORY: Set to our ./lib folder
    set_target_properties(${LIB_TARGET_NAME} PROPERTIES
            PREFIX ""
            LIBRARY_OUTPUT_DIRECTORY ${MODULE_OUTPUT_DIR}
    )

    # Make sure the module can find the core headers
    target_include_directories(${LIB_TARGET_NAME} PRIVATE
            ${PROJECT_SOURCE_DIR}/src
            ${PROJECT_SOURCE_DIR}/src/base-lib
    )

    # Conditional Linking (for 'random' only)
    if(LIB_TARGET_NAME STREQUAL "random")
        message(STATUS "Linking ${LIB_TARGET_NAME} module against OpenSSL.")
        target_link_libraries(${LIB_TARGET_NAME} PRIVATE
                OpenSSL::SSL
                #OpenSSL::Crypto
        )
    endif()

    # Add export-dynamic flag on Linux/BSD
    if(UNIX AND NOT APPLE)
        message(STATUS "Adding -Wl,--export-dynamic to main executable for plugin support.")
        target_link_options(cozenage PRIVATE -Wl,--export-dynamic)
    endif()

    # Add platform-specific linker flags for modules
    if(APPLE)
        # Tell macOS linker to allow undefined symbols...
        message(STATUS "Building module ${LIB_TARGET_NAME} with -Wl,-undefined,dynamic_lookup")
        set_target_properties(${LIB_TARGET_NAME} PROPERTIES
                LINK_FLAGS "-Wl,-undefined,dynamic_lookup"
        )
    endif()
endforeach()
