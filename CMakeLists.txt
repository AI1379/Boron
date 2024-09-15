cmake_minimum_required(VERSION 3.14)
project(Boron VERSION 0.1.0)

if(WIN32)
  set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
  cmake_policy(SET CMP0091 NEW)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Werror")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O3")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(BORON_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(BORON_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(BORON_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(BORON_MODULE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

list(APPEND CMAKE_MODULE_PATH ${BORON_MODULE_DIR})

add_subdirectory(src)

if(BORON_TEST_ENABLED)
  enable_testing()
  add_subdirectory(test)
endif()
