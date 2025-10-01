cmake_minimum_required(VERSION 3.10)

# here we specify that the project is C language only, so the default
# C compiler on the system will be used
project(cozenage C)
set(CMAKE_C_STANDARD 23)

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

add_executable(cozenage src/main.c
        src/main.h
        src/compat_readline.h
        src/parser.h
        src/parser.c
        src/printer.h
        src/printer.c
        src/types.c
        src/types.h
        src/environment.c
        src/environment.h
        src/ops.c
        src/ops.h
        src/eval.c
        src/eval.h
        src/file_lib.h
        src/file_lib.c
        src/coz_ext_lib.h
        src/coz_ext_lib.c
        src/char_lib.c
        src/char_lib.h
        src/complex_lib.c
        src/complex_lib.h
        src/process_context_lib.c
        src/process_context_lib.h
        src/inexact_lib.c
        src/inexact_lib.h
        src/load_library.c
        src/load_library.h
        src/io_lib.c
        src/io_lib.h
        src/ports.c
        src/ports.h
        src/strings.c
        src/strings.h
        src/chars.c
        src/chars.h
        src/special_forms.c
        src/special_forms.h
        src/comparators.c
        src/comparators.h
        src/bytevectors.c
        src/bytevectors.h
        src/vectors.c
        src/vectors.h
        src/pairs.c
        src/pairs.h
        src/predicates.c
        src/predicates.h
        src/bools.c
        src/bools.h
        src/eval_lib.c
        src/eval_lib.h)

# Find math library
find_library(M_LIB m)
find_library(GC_LIB gc)

target_link_libraries(cozenage PRIVATE ${READLINE_LIBRARY} ${M_LIB} ${GC_LIB})

