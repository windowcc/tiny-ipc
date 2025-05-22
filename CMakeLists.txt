cmake_minimum_required(VERSION 3.16.3)
cmake_policy(VERSION 3.16.3...3.20.3)
project(tiny-ipc)

set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)

option(LIBIPC_BUILD_TESTS       "Build all of libipc's own tests."                      OFF)
option(LIBIPC_BUILD_SAMPLES     "Build all of libipc's own samples."                      ON)
option(LIBIPC_BUILD_SHARED_LIBS "Build shared libraries (DLLs)."                        ON)
option(LIBIPC_USE_STATIC_CRT    "Set to ON to build with static CRT on Windows (/MT)."  OFF)

set(MAX_CONNECTIONS 64)

configure_file (
    "${PROJECT_SOURCE_DIR}/config.h.in"
    "${PROJECT_BINARY_DIR}/config.h"
)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG")
if(NOT MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")
endif()

if (MSVC)
    set(CompilerFlags
        CMAKE_CXX_FLAGS
        CMAKE_CXX_FLAGS_DEBUG
        CMAKE_CXX_FLAGS_RELEASE
        CMAKE_C_FLAGS
        CMAKE_C_FLAGS_DEBUG
        CMAKE_C_FLAGS_RELEASE
        )
    if (LIBIPC_USE_STATIC_CRT)
        foreach(CompilerFlag ${CompilerFlags})
            string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
            string(REPLACE "/MDd" "/MTd" ${CompilerFlag} "${${CompilerFlag}}")
        endforeach()
    else()
        foreach(CompilerFlag ${CompilerFlags})
            string(REPLACE "/MT" "/MD" ${CompilerFlag} "${${CompilerFlag}}")
            string(REPLACE "/MTd" "/MDd" ${CompilerFlag} "${${CompilerFlag}}")
        endforeach()
    endif()
endif()


# Unicode Support
add_definitions(-DUNICODE -D_UNICODE)

add_subdirectory(src)

if (LIBIPC_BUILD_TESTS)
    set(GOOGLETEST_VERSION 1.10.0)
    if (LIBIPC_USE_STATIC_CRT)
        set(gtest_force_shared_crt OFF)
    else()
        set(gtest_force_shared_crt ON)
    endif()
    add_subdirectory(3rdparty/gtest)
    add_subdirectory(test)
endif()

if (LIBIPC_BUILD_SAMPLES)
    add_subdirectory(examples/channel)
    add_subdirectory(examples/route)
endif()