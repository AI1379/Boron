cmake_minimum_required(VERSION 3.14)
project(Boron VERSION 0.1.0)

message(STATUS "$ENV{PATH}")
message(STATUS "C++ compiler id: ${CMAKE_CXX_COMPILER_ID}")

if (WIN32)
  message(STATUS "Configuring for Windows")
  set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
  cmake_policy(SET CMP0091 NEW)
  set(CMAKE_CXX_FLAGS_DEBUG
      "${CMAKE_CXX_FLAGS_DEBUG} -D_ITERATOR_DEBUG_LEVEL=2 -MTd")
  set(CMAKE_CXX_FLAGS_RELEASE
      "${CMAKE_CXX_FLAGS_RELEASE} -D_ITERATOR_DEBUG_LEVEL=0 -MT")
elseif (UNIX AND NOT APPLE)
  message(STATUS "Configuring for Linux")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_GLIBCXX_DEBUG")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
elseif (APPLE)
  message(STATUS "Configuring for macOS")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_LIBCPP_DEBUG=1")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
else ()
  message(FATAL_ERROR "Unsupported platform")
endif ()

option(BORON_TEST_ENABLED "Enable testing" ON)
option(BORON_ENABLE_ASAN "Enable AddressSanitizer" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS_DEBUG
    "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG} -Wall"
)
if (NOT MSVC)
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG} -Wextra -Wpedantic -Werror")
endif ()
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE} -O3")
if (BORON_ENABLE_ASAN)
  # TODO: check why clang complain about -fsanitize=address when build on Windows
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fsanitize=address")
endif ()
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(BORON_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(BORON_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(BORON_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(BORON_MODULE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

list(APPEND CMAKE_MODULE_PATH ${BORON_MODULE_DIR})

add_subdirectory(src)

if (BORON_TEST_ENABLED)
  add_subdirectory(test)
endif ()
