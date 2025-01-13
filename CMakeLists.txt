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

add_executable(
    ${PROJECT_NAME}
    main.cc
    interpreter/interpreter.cc
)
target_link_libraries(${PROJECT_NAME} PRIVATE parser)
