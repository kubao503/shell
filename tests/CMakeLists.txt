enable_testing()

add_executable(
    test
    test_parser.cc
)
target_link_libraries(
    test
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(test)
