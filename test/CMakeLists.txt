enable_testing()

include(GoogleTest)
find_package(GTest REQUIRED)

option(BORON_USE_OWN_TEST_MAIN "Use own test main" OFF)

set(TEST_SOURCES ByteArrayTest.cpp
    TestMain.cpp)

message(STATUS "GTest libraries: ${GTEST_LIBRARIES}")
message(STATUS "GTest main libraries: ${GTEST_MAIN_LIBRARIES}")
message(STATUS "Test Sources: ${TEST_SOURCES}")

add_executable(BoronTest ${TEST_SOURCES})
if (BORON_USE_OWN_TEST_MAIN)
  target_link_libraries(BoronTest Boron GTest::gtest)
else ()
  target_link_libraries(BoronTest Boron GTest::gtest_main)
endif ()
gtest_discover_tests(BoronTest)
