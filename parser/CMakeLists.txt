find_package(FLEX)
find_package(BISON)

set(LEXER_OUT "${CMAKE_CURRENT_BINARY_DIR}/lexer.cc")
set(PARSER_OUT "${CMAKE_CURRENT_BINARY_DIR}/parser.cc")

flex_target(LEXER "${CMAKE_CURRENT_SOURCE_DIR}/lexer.ll" "${LEXER_OUT}")
bison_target(PARSER "${CMAKE_CURRENT_SOURCE_DIR}/parser.yy" "${PARSER_OUT}")
add_flex_bison_dependency(LEXER PARSER)

add_library(parser "${LEXER_OUT}" "${PARSER_OUT}")

target_include_directories(parser PUBLIC "${CMAKE_CURRENT_BINARY_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")
