cmake_minimum_required(VERSION 3.12)
project(coolc)

option(ASAN "Build with AddressSanitizer" OFF)
option(UBSAN "Build with UndefinedBehaviorSanitizer" OFF)
set(ARCH "MIPS" CACHE STRING "Target architecture")

execute_process (
    COMMAND sh -c "rm -rf ${PROJECT_SOURCE_DIR}/bin"
)

execute_process (
    COMMAND sh -c "rm -rf ${PROJECT_SOURCE_DIR}/lib"
)

if(ARCH STREQUAL "LLVM")
    execute_process(COMMAND sh -c "llvm-config --includedir" OUTPUT_VARIABLE LLVM_INCLUDE_DIR)
    string(STRIP ${LLVM_INCLUDE_DIR} LLVM_INCLUDE_DIR)
    message(STATUS "LLVM_INCLUDE_DIR = ${LLVM_INCLUDE_DIR}")
    include_directories(${LLVM_INCLUDE_DIR})

    execute_process(COMMAND sh -c "llvm-config --libs all" OUTPUT_VARIABLE LIBS)
    string(STRIP ${LIBS} LIBS)
    message(STATUS "LIBS = ${LIBS}")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_CLANG_TIDY "clang-tidy;--extra-arg=-std=gnu++2a")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-switch -D${ARCH}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG -D${ARCH}")

if(ASAN)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address -fno-omit-frame-pointer")
endif()

if(UBSAN)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=undefined -fno-omit-frame-pointer")
endif()

message(STATUS "FRONTEND CMAKE_CXX_FLAGS = ${CMAKE_CXX_FLAGS}")
message(STATUS "FRONTEND CMAKE_CXX_FLAGS_DEBUG = ${CMAKE_CXX_FLAGS_DEBUG}")

unset(ASAN CACHE)
unset(UBSAN CACHE)

include_directories(src)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

add_subdirectory(src/lexer)
add_subdirectory(src/parser)
add_subdirectory(src/semant)
add_subdirectory(src/utils)
add_subdirectory(src/ast)
add_subdirectory(src/decls)
add_subdirectory(src/codegen)
add_subdirectory(src/codegen/runtime)
add_executable(coolc src/coolc.cpp)

target_link_libraries(coolc lexer parser semant utils ast codegen decls ${LIBS})

enable_testing()

if(ARCH STREQUAL "MIPS")
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")

        execute_process (
            COMMAND sh -c "rm ${PROJECT_SOURCE_DIR}/tests/codegen/mips/end-to-end/*.s"
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
endif()