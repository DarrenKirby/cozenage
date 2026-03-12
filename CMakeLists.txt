cmake_minimum_required(VERSION 3.10)
project(Cozenage LANGUAGES C VERSION 0.16.0)
set(CMAKE_C_STANDARD 23)

message(STATUS "Building ${PROJECT_NAME} v${PROJECT_VERSION}")

# --- Build flags ---
option(DEBUG_BUILD "Enable debug CFLAGS" OFF)
if(DEBUG_BUILD)
    set(CMAKE_C_FLAGS "-march=native -Wall -Wextra -Werror -O0 -g -frounding-math -fsigned-zeros")
    message(STATUS "Building with debugging symbols enabled.")
else()
    set(CMAKE_C_FLAGS "-march=native -Wall -Wextra -Werror -O2 -frounding-math -fsigned-zeros")
endif()
message(STATUS "Building with CFLAGS: ${CMAKE_C_FLAGS}")

# --- Platform search paths ---
# Homebrew (macOS) and FreeBSD both put third-party libraries under a prefix
# that CMake doesn't search by default. Setting CMAKE_PREFIX_PATH fixes this
# for all subsequent find_package/find_library calls at once.
if(APPLE)
    list(APPEND CMAKE_PREFIX_PATH /opt/homebrew /usr/local)
elseif(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
    list(APPEND CMAKE_PREFIX_PATH /usr/local)
endif()

find_package(PkgConfig QUIET)

# --- Required libraries ---

# ICU: standard find_package first, fall back to pkg-config
find_package(ICU COMPONENTS uc QUIET)
if(NOT ICU_FOUND AND PKG_CONFIG_FOUND)
    message(STATUS "CMake FindICU failed, falling back to pkg-config...")
    pkg_check_modules(ICU QUIET IMPORTED_TARGET icu-uc)
    if(ICU_FOUND)
        # Alias the pkg-config target to the canonical name
        add_library(ICU::uc INTERFACE IMPORTED)
        target_link_libraries(ICU::uc INTERFACE PkgConfig::ICU)
    endif()
endif()
if(NOT ICU_FOUND)
    message(FATAL_ERROR "Could not find ICU (icu-uc). Install it via your package manager.")
endif()

find_library(GC_LIB gc)
if(NOT GC_LIB)
    message(FATAL_ERROR "Could not find libgc (Boehm GC).")
endif()

find_library(GMP_LIB gmp)
if(NOT GMP_LIB)
    message(FATAL_ERROR "Could not find libgmp.")
endif()

find_library(M_LIB m)  # May be absent (folded into libc on some systems)

# libdl: Linux has it as a separate library; FreeBSD folds it into libc.
find_library(DL_LIB dl)
if(NOT DL_LIB AND NOT CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
    message(FATAL_ERROR "Could not find libdl.")
endif()

find_package(OpenSSL QUIET)
if(OpenSSL_FOUND)
    message(STATUS "Found OpenSSL ${OPENSSL_VERSION} — 'random' module will be built.")
else()
    message(STATUS "OpenSSL not found — 'random' module will NOT be built.")
endif()

# --- Main executable ---
file(GLOB COZENAGE_SOURCES "src/*.c")
list(REMOVE_ITEM COZENAGE_SOURCES "${PROJECT_SOURCE_DIR}/src/main.c")

add_executable(cozenage src/main.c ${COZENAGE_SOURCES})

target_include_directories(cozenage PRIVATE
        ${PROJECT_SOURCE_DIR}/src
        ${PROJECT_SOURCE_DIR}/src/base-lib
)

target_link_libraries(cozenage PRIVATE
        ${GC_LIB}
        ${GMP_LIB}
        ICU::uc
)
if(M_LIB)
    target_link_libraries(cozenage PRIVATE ${M_LIB})
endif()
if(DL_LIB)
    target_link_libraries(cozenage PRIVATE ${DL_LIB})
endif()

target_compile_definitions(cozenage PUBLIC APP_VERSION="${PROJECT_VERSION}")

# Platform linker flags — set ONCE here, not inside the module loop
if(APPLE)
    target_link_options(cozenage PRIVATE "-Wl,-rpath,@executable_path/../lib/cozenage/")
elseif(UNIX)
    # --export-dynamic makes main binary's symbols visible to dlopen'd modules
    target_link_options(cozenage PRIVATE
            -Wl,--export-dynamic
            "-Wl,-rpath,\$ORIGIN/../lib/cozenage"
    )
endif()

# --- Loadable modules ---
set(MODULE_OUTPUT_DIR ${PROJECT_SOURCE_DIR}/lib)
file(MAKE_DIRECTORY ${MODULE_OUTPUT_DIR})
file(GLOB LIB_SOURCES "src/base-lib/*.c")

foreach(LIB_SOURCE ${LIB_SOURCES})
    get_filename_component(LIB_NAME ${LIB_SOURCE} NAME_WE)
    string(REPLACE "_lib" "" LIB_TARGET_NAME ${LIB_NAME})

    if(LIB_TARGET_NAME STREQUAL "random" AND NOT OpenSSL_FOUND)
        continue()
    endif()

    add_library(${LIB_TARGET_NAME} SHARED ${LIB_SOURCE})

    target_include_directories(${LIB_TARGET_NAME} PRIVATE
            ${PROJECT_SOURCE_DIR}/src
            ${PROJECT_SOURCE_DIR}/src/base-lib
    )

    target_link_libraries(${LIB_TARGET_NAME} PRIVATE ICU::uc)

    set_target_properties(${LIB_TARGET_NAME} PROPERTIES
            PREFIX ""
            LIBRARY_OUTPUT_DIRECTORY ${MODULE_OUTPUT_DIR}
    )

    if(LIB_TARGET_NAME STREQUAL "random")
        target_link_libraries(${LIB_TARGET_NAME} PRIVATE OpenSSL::SSL)
    endif()

    if(APPLE)
        set_target_properties(${LIB_TARGET_NAME} PROPERTIES
                LINK_FLAGS "-Wl,-undefined,dynamic_lookup"
        )
    endif()
endforeach()
