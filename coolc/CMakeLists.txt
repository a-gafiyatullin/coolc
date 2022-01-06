cmake_minimum_required(VERSION 3.12)
project(coolc)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_CLANG_TIDY "clang-tidy")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")

include_directories(src)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

add_subdirectory(src/lexer)
add_subdirectory(src/parser)
add_subdirectory(src/semant)
add_subdirectory(src/utils)
add_subdirectory(src/codegen/mips)
add_subdirectory(src/ast)

add_executable(coolc src/coolc.cpp)
target_link_libraries(coolc lexer parser semant utils ast codegen)