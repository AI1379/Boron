message(
    STATUS "CMakeLists.txt: CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}"
)

set(BORON_SOURCES ${BORON_SOURCE_DIR}/ByteArray.cpp
    ${BORON_SOURCE_DIR}/ByteArrayAlgorithms.cpp)

include_directories(${BORON_INCLUDE_DIR})

message(STATUS "CMakeLists.txt: BORON_SOURCES: ${BORON_SOURCES}")

option(BORON_ENABLE_GMP "Enable GMP" OFF)

if (BORON_ENABLE_GMP)
  find_package(GMP REQUIRED)
  include_directories(${GMP_INCLUDE_DIR})
  add_definitions(-DBORON_ENABLE_GMP)
  set(BORON_SOURCES ${BORON_SOURCES} ${BORON_SOURCE_DIR}/GmpBigInt.cpp)
endif ()

add_library(Boron ${BORON_SOURCES})
target_include_directories(Boron PUBLIC ${BORON_INCLUDE_DIR})
