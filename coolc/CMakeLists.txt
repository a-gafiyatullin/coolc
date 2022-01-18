cmake_minimum_required(VERSION 3.12)
project(coolc)

option(ASAN "Build with AddressSanitizer" OFF)
option(UBSAN "Build with UndefinedBehaviorSanitizer" OFF)

execute_process (
    COMMAND bash -c "rm -rf ${PROJECT_SOURCE_DIR}/bin"
)

execute_process (
    COMMAND bash -c "rm -rf ${PROJECT_SOURCE_DIR}/lib"
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_CLANG_TIDY "clang-tidy;--extra-arg=-std=gnu++2a")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-switch")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")

if(ASAN)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address -fno-omit-frame-pointer")
endif()

if(UBSAN)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=undefined -fno-omit-frame-pointer")
endif()

unset(ASAN CACHE)
unset(UBSAN CACHE)

include_directories(src)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

add_subdirectory(src/lexer)
add_subdirectory(src/parser)
add_subdirectory(src/semant)
add_subdirectory(src/utils)
add_subdirectory(src/codegen/mips)
add_subdirectory(src/ast)

add_executable(coolc src/coolc.cpp)
target_link_libraries(coolc lexer parser semant utils ast codegen)

enable_testing()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")

    execute_process (
        COMMAND bash -c "rm ${PROJECT_SOURCE_DIR}/tests/codegen/mips/end-to-end/*.s"
    )

    add_test(
        NAME Lexer
        COMMAND python3
        ${PROJECT_SOURCE_DIR}/tests/e2e.py
        ${PROJECT_SOURCE_DIR}/tests/reference/bin/lexer
        ""
        ${PROJECT_SOURCE_DIR}/bin/coolc
        "+TokensOnly"
        ${PROJECT_SOURCE_DIR}/tests/lexer/end-to-end/
        ".cool")

    add_test(
        NAME ParserSemant
        COMMAND python3
        ${PROJECT_SOURCE_DIR}/tests/e2e.py
        ${PROJECT_SOURCE_DIR}/tests/parser-semant/reference-coolc.sh
        "${PROJECT_SOURCE_DIR}/tests/reference/bin"
        ${PROJECT_SOURCE_DIR}/bin/coolc
        "+PrintFinalAST"
        ${PROJECT_SOURCE_DIR}/tests/parser-semant/end-to-end/
        ".test")

    add_test(
        NAME CodeGen
        COMMAND python3
        ${PROJECT_SOURCE_DIR}/tests/e2e.py
        ${PROJECT_SOURCE_DIR}/tests/codegen/mips/start-spim.sh
        "${PROJECT_SOURCE_DIR}/tests/reference/bin ${PROJECT_SOURCE_DIR}/tests/reference/bin -g"
        ${PROJECT_SOURCE_DIR}/tests/codegen/mips/start-spim.sh
        "${PROJECT_SOURCE_DIR}/tests/reference/bin ${PROJECT_SOURCE_DIR}/bin"
        ${PROJECT_SOURCE_DIR}/tests/codegen/mips/end-to-end/
        ".cl")
endif()