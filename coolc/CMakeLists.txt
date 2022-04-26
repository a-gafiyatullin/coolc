cmake_minimum_required(VERSION 3.12)
project(coolc)

option(ASAN "Build with AddressSanitizer" OFF)
option(UBSAN "Build with UndefinedBehaviorSanitizer" OFF)
set(ARCH "MIPS" CACHE STRING "Target architecture")
set(ARCH "NATIVE" CACHE STRING "Build for Host cpu and Host features")

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

    execute_process(COMMAND sh -c "llvm-config --libdir" OUTPUT_VARIABLE LIB_DIR)
    string(STRIP ${LIB_DIR} LIB_DIR)
    message(STATUS "LIB_DIR = ${LIB_DIR}")

    link_directories(${LIB_DIR})
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

if(NATIVE)
    message(STATUS "Build in NATIVE mode!")

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNATIVE")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DNATIVE")
endif()

message(STATUS "CMAKE_CXX_FLAGS = ${CMAKE_CXX_FLAGS}")
message(STATUS "CMAKE_CXX_FLAGS_DEBUG = ${CMAKE_CXX_FLAGS_DEBUG}")

unset(ASAN CACHE)
unset(UBSAN CACHE)
unset(NATIVE CACHE)

include_directories(src)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

add_subdirectory(src/lexer)
add_subdirectory(src/parser)
add_subdirectory(src/semant)
add_subdirectory(src/utils)
add_subdirectory(src/ast)
add_subdirectory(src/decls)
add_subdirectory(src/codegen)
add_executable(coolc src/coolc.cpp)

target_link_libraries(coolc lexer parser semant utils ast codegen decls ${LIBS})

# Build runtime lib. Allow ClassNameTab to be undefined.
if(APPLE)
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-U,_ClassNameTab, -Wl,-U,_BoolTag, -Wl,-U,_IntTag, -Wl,-U,_StringTag")
    set(CMAKE_SHARED_LIBRARY_SUFFIX .so)
endif()
add_subdirectory(src/codegen/runtime)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    enable_testing()

    include_directories(tests/src)

    find_package(GTest REQUIRED)
    include_directories(${GTEST_INCLUDE_DIRS})

    add_test(PrepareLexerTestsResults
        ${PROJECT_SOURCE_DIR}/tests/lexer/make_results.sh
        ${EXECUTABLE_OUTPUT_PATH}
        )
    add_executable(lexer_tests tests/src/test.cpp tests/src/lexer/test.cpp)
    target_link_libraries(lexer_tests ${GTEST_LIBRARIES} pthread ${GTEST_MAIN_LIBRARIES})
    add_test(LexerTests ${EXECUTABLE_OUTPUT_PATH}/lexer_tests)

    add_test(PrepareParserSemantTestsResults
        ${PROJECT_SOURCE_DIR}/tests/parser-semant/make_results.sh
        ${EXECUTABLE_OUTPUT_PATH}
        )
    add_executable(parser_semant_tests tests/src/test.cpp tests/src/parser-semant/test.cpp)
    target_link_libraries(parser_semant_tests ${GTEST_LIBRARIES} pthread ${GTEST_MAIN_LIBRARIES})
    add_test(ParserSemantTests ${EXECUTABLE_OUTPUT_PATH}/parser_semant_tests)

    if(ARCH STREQUAL "MIPS")
        set(RUN_DIR mips)
    else()
        set(RUN_DIR llvm)
    endif()

    if ((CMAKE_SYSTEM_PROCESSOR MATCHES "(x86)|(X86)|(amd64)|(AMD64)" AND ARCH STREQUAL "MIPS") OR ARCH STREQUAL "LLVM")
        add_test(PrepareCodegenTestsResults
            ${PROJECT_SOURCE_DIR}/tests/codegen/make_results.sh
            ${EXECUTABLE_OUTPUT_PATH}
            ${PROJECT_SOURCE_DIR}/tests/codegen/arch/${RUN_DIR}/run.sh
            )
        add_executable(codegen_tests tests/src/test.cpp tests/src/codegen/test.cpp)
        target_link_libraries(codegen_tests ${GTEST_LIBRARIES} pthread ${GTEST_MAIN_LIBRARIES})
        add_test(CodegenTests ${EXECUTABLE_OUTPUT_PATH}/codegen_tests)
    endif()
endif()