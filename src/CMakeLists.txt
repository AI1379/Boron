message(
  STATUS "CMakeLists.txt: CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}"
)

set(BORON_SOURCES ${BORON_SOURCE_DIR}/ByteArray.cpp)

include_directories(${BORON_INCLUDE_DIR})

message(STATUS "CMakeLists.txt: BORON_SOURCES: ${BORON_SOURCES}")

add_library(Boron ${BORON_SOURCES})
target_include_directories(Boron PUBLIC ${BORON_INCLUDE_DIR})
