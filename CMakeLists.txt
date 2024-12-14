cmake_minimum_required(VERSION 3.14)
project(shell)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/79219e26e0e36b415a5804b6b017ad6c6cd99ad8.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_subdirectory(parser)
add_subdirectory(tests)

set(SRC_DIR parser)
set(PARSER_DIR "${CMAKE_CURRENT_BINARY_DIR}")

find_package(FLEX 2.6 REQUIRED)
find_package(BISON 3.0 REQUIRED)

set(LEXER_OUT "${PARSER_DIR}/parser/lexer.cc")
set(PARSER_OUT "${PARSER_DIR}/parser/parser.cc")

flex_target(LEXER "${SRC_DIR}/fooLexer.ll" "${LEXER_OUT}")
bison_target(PARSER "${SRC_DIR}/fooParser.yy" "${PARSER_OUT}")
add_flex_bison_dependency(LEXER PARSER)

add_executable(
    ${PROJECT_NAME}
    main.cc
    interpreter.cc
    "${LEXER_OUT}"
    "${PARSER_OUT}"
)
target_include_directories(${PROJECT_NAME} PRIVATE "${PARSER_DIR}" parser)
