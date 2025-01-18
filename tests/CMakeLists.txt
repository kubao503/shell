enable_testing()

add_executable(
    test
    test_parser.cc
    test_word_expansion.cc
    ../interpreter/interpreter.cc
    ../interpreter/util.cc
)

target_link_libraries(
    test
    GTest::gtest_main
    parser
)

include(GoogleTest)
gtest_discover_tests(test)
