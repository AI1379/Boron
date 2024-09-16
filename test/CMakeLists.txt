enable_testing()

include(GoogleTest)
find_package(GTest REQUIRED)

set(TEST_SOURCES ByteArrayTest.cpp)

message(STATUS "GTest libraries: ${GTEST_LIBRARIES}")
message(STATUS "GTest main libraries: ${GTEST_MAIN_LIBRARIES}")
message(STATUS "Test Sources: ${TEST_SOURCES}")

add_executable(BoronTest ${TEST_SOURCES})
target_link_libraries(BoronTest Boron GTest::gtest_main)
gtest_discover_tests(BoronTest)
