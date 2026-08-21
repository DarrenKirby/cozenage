cmake_minimum_required(VERSION 3.16)

project(Cozenage
        VERSION 0.19.1
        LANGUAGES C
)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

message(STATUS "Building ${PROJECT_NAME} v${PROJECT_VERSION}")

#
# Build options
#

option(DEBUG_BUILD "Build with debugging information" OFF)

add_library(cozenage_options INTERFACE)

if (DEBUG_BUILD)
    message(STATUS "Debug build enabled.")
    target_compile_options(cozenage_options INTERFACE
            -march=native
            -Wall
            -Wextra
            -Werror
            -O0
            -g
            -frounding-math
            -fsigned-zeros
    )
else ()
    target_compile_options(cozenage_options INTERFACE
            -march=native
            -Wall
            -Wextra
            -Werror
            -O2
            -frounding-math
            -fsigned-zeros
    )
endif ()

#
# Platform search paths
#

if (APPLE)
    list(APPEND CMAKE_PREFIX_PATH
            /opt/homebrew
            /usr/local
    )
elseif (CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
    list(APPEND CMAKE_PREFIX_PATH
            /usr/local
    )
endif ()

#
# Find dependencies
#

find_package(PkgConfig REQUIRED)

pkg_check_modules(GMP REQUIRED IMPORTED_TARGET gmp)
pkg_check_modules(GC REQUIRED IMPORTED_TARGET bdw-gc)

find_package(ICU COMPONENTS uc QUIET)
if (NOT ICU_FOUND AND PKG_CONFIG_FOUND)
    message(STATUS "CMake FindICU failed, falling back to pkg-config...")
    pkg_check_modules(ICU QUIET IMPORTED_TARGET icu-uc)

    if (ICU_FOUND)
        add_library(ICU::uc INTERFACE IMPORTED)
        target_link_libraries(ICU::uc INTERFACE PkgConfig::ICU)
    endif ()
endif ()

if (NOT ICU_FOUND)
    message(FATAL_ERROR "Could not find ICU.")
endif ()

find_library(M_LIB m)

find_package(OpenSSL QUIET)

if (OpenSSL_FOUND)
    message(STATUS "Found OpenSSL ${OPENSSL_VERSION} — random module will be built.")
else ()
    message(STATUS "OpenSSL not found — random module will NOT be built.")
endif ()

#
# Common project configuration
#

add_library(cozenage_common INTERFACE)

target_link_libraries(cozenage_common INTERFACE
        cozenage_options
        PkgConfig::GMP
        PkgConfig::GC
        ICU::uc
)

target_include_directories(cozenage_common INTERFACE
        ${PROJECT_SOURCE_DIR}/src
        ${PROJECT_SOURCE_DIR}/src/base-lib
)

#
# Main executable
#

file(GLOB COZENAGE_SOURCES
        CONFIGURE_DEPENDS
        "${PROJECT_SOURCE_DIR}/src/*.c"
)

list(REMOVE_ITEM COZENAGE_SOURCES
        "${PROJECT_SOURCE_DIR}/src/main.c"
)

add_executable(cozenage
        src/main.c
        ${COZENAGE_SOURCES}
)

target_link_libraries(cozenage PRIVATE
        cozenage_common
        ${CMAKE_DL_LIBS}
)

if (M_LIB)
    target_link_libraries(cozenage PRIVATE
            ${M_LIB}
    )
endif ()

target_compile_definitions(cozenage PRIVATE
        APP_VERSION="${PROJECT_VERSION}"
)

if (APPLE)

    target_link_options(cozenage PRIVATE
            "-Wl,-rpath,@executable_path/../lib/cozenage/base"
    )

elseif (UNIX)

    target_link_options(cozenage PRIVATE
            -Wl,--export-dynamic
            "-Wl,-rpath,\$ORIGIN/../lib/cozenage/base"
    )

endif ()

#
# Loadable modules
#

set(MODULE_OUTPUT_DIR
        ${PROJECT_SOURCE_DIR}/lib/cozenage/base
)

file(GLOB MODULE_SOURCES
        CONFIGURE_DEPENDS
        "${PROJECT_SOURCE_DIR}/src/base-lib/*.c"
)

foreach (MODULE_SOURCE ${MODULE_SOURCES})

    get_filename_component(MODULE_NAME
            ${MODULE_SOURCE}
            NAME_WE
    )

    string(REPLACE "_lib" ""
            MODULE_TARGET_NAME
            ${MODULE_NAME}
    )

    if (MODULE_TARGET_NAME STREQUAL "random"
            AND NOT OpenSSL_FOUND)
        continue()
    endif ()

    add_library(${MODULE_TARGET_NAME}
            SHARED
            ${MODULE_SOURCE}
    )

    target_link_libraries(${MODULE_TARGET_NAME}
            PRIVATE
            cozenage_common
    )

    #
    # Module-specific dependencies
    #

    if (MODULE_TARGET_NAME STREQUAL "random")

        target_link_libraries(${MODULE_TARGET_NAME}
                PRIVATE
                OpenSSL::SSL
                OpenSSL::Crypto
        )

    endif ()

    # Example:
    #
    # if(MODULE_TARGET_NAME STREQUAL "math")
    #     target_link_libraries(${MODULE_TARGET_NAME}
    #         PRIVATE
    #             ${M_LIB}
    #     )
    # endif()

    set_target_properties(${MODULE_TARGET_NAME}
            PROPERTIES
            PREFIX ""
            LIBRARY_OUTPUT_DIRECTORY ${MODULE_OUTPUT_DIR}
    )

    if (APPLE)

        set_target_properties(${MODULE_TARGET_NAME}
                PROPERTIES
                LINK_FLAGS "-Wl,-undefined,dynamic_lookup"
        )

    endif ()

endforeach ()
