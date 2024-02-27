cmake_minimum_required(VERSION 3.12)
project(coolc)

option(ASAN "Build with AddressSanitizer" OFF)
option(UBSAN "Build with UndefinedBehaviorSanitizer" OFF)
set(ARCH "LLVM" CACHE STRING "Target architecture")
set(GCTYPE "LLVM_STATEPOINT_EXAMPLE" CACHE STRING "GC")

execute_process(
  COMMAND sh -c "rm -rf ${PROJECT_SOURCE_DIR}/bin"
)

execute_process(
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

  if(GCTYPE STREQUAL "LLVM_SHADOW_STACK")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DLLVM_SHADOW_STACK")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DLLVM_SHADOW_STACK")
  elseif(GCTYPE STREQUAL "LLVM_STATEPOINT_EXAMPLE")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DLLVM_STATEPOINT_EXAMPLE")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DLLVM_STATEPOINT_EXAMPLE")
  endif()
else()
  set(GCTYPE "NO_GC")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_CLANG_TIDY "clang-tidy;--extra-arg=-std=gnu++2a")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-switch -D${ARCH}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG -D${ARCH}")

if(ASAN)
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DASAN -fsanitize=address -fno-omit-frame-pointer")
endif()

if(UBSAN)
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DUBSAN -fsanitize=undefined -fno-omit-frame-pointer")
endif()

message(STATUS "CMAKE_CXX_FLAGS = ${CMAKE_CXX_FLAGS}")
message(STATUS "CMAKE_CXX_FLAGS_DEBUG = ${CMAKE_CXX_FLAGS_DEBUG}")

unset(ASAN CACHE)
unset(UBSAN CACHE)

include_directories(src)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

if(ARCH STREQUAL "LLVM")
  find_package(Boost 1.61 COMPONENTS filesystem REQUIRED)
  include_directories(${Boost_INCLUDE_DIR})

  set(ADDITIONAL_LIBS "${Boost_LIBRARIES}")
  message(STATUS "ADDITIONAL_LIBS = ${ADDITIONAL_LIBS}")
endif()

add_subdirectory(src/lexer)
add_subdirectory(src/parser)
add_subdirectory(src/semant)
add_subdirectory(src/utils)
add_subdirectory(src/ast)
add_subdirectory(src/decls)
add_subdirectory(src/codegen)
add_executable(coolc src/coolc.cpp)

target_link_libraries(coolc lexer parser semant utils ast codegen decls ${LIBS} ${ADDITIONAL_LIBS} -ldl)

if(ARCH STREQUAL "LLVM")
  if(APPLE)
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -undefined dynamic_lookup")
    set(CMAKE_SHARED_LIBRARY_SUFFIX .so)
  endif()

  add_subdirectory(src/runtime)
endif()

enable_testing()

include_directories(tests/src)

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
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
endif()

if(ARCH STREQUAL "MIPS")
  set(RUN_DIR mips)
else()
  set(RUN_DIR llvm)
endif()

add_executable(codegen_tests tests/src/test.cpp tests/src/codegen/test.cpp)
target_link_libraries(codegen_tests ${GTEST_LIBRARIES} pthread ${GTEST_MAIN_LIBRARIES})

if(CMAKE_SYSTEM_PROCESSOR MATCHES "(x86)|(X86)|(amd64)|(AMD64)" AND ARCH STREQUAL "MIPS")
  add_test(PrepareCodegenTestsResults
    ${PROJECT_SOURCE_DIR}/tests/codegen/make_results.sh
    ${EXECUTABLE_OUTPUT_PATH}
    ${PROJECT_SOURCE_DIR}/tests/codegen/arch/${RUN_DIR}/run.sh
  )
  add_test(CodegenTests ${EXECUTABLE_OUTPUT_PATH}/codegen_tests)
elseif(ARCH STREQUAL "LLVM")
  add_test(PrepareCodegenMarkAndSweepTestsResults
    ${PROJECT_SOURCE_DIR}/tests/codegen/make_results.sh
    ${EXECUTABLE_OUTPUT_PATH}
    ${PROJECT_SOURCE_DIR}/tests/codegen/arch/${RUN_DIR}/run.sh
    1
    "6Kb"
  )
  add_test(CodegenTestsMarkAndSweep ${EXECUTABLE_OUTPUT_PATH}/codegen_tests)

  add_test(PrepareCodegenThreadedTestsResults
    ${PROJECT_SOURCE_DIR}/tests/codegen/make_results.sh
    ${EXECUTABLE_OUTPUT_PATH}
    ${PROJECT_SOURCE_DIR}/tests/codegen/arch/${RUN_DIR}/run.sh
    2
    "5Kb"
  )
  add_test(CodegenTestsThreaded ${EXECUTABLE_OUTPUT_PATH}/codegen_tests)

  add_test(PrepareCodegenCompressorTestsResults
    ${PROJECT_SOURCE_DIR}/tests/codegen/make_results.sh
    ${EXECUTABLE_OUTPUT_PATH}
    ${PROJECT_SOURCE_DIR}/tests/codegen/arch/${RUN_DIR}/run.sh
    3
    "5Kb"
  )
  add_test(CodegenTestsCompressor ${EXECUTABLE_OUTPUT_PATH}/codegen_tests)

  add_test(PrepareCodegenSemispaceTestsResults
    ${PROJECT_SOURCE_DIR}/tests/codegen/make_results.sh
    ${EXECUTABLE_OUTPUT_PATH}
    ${PROJECT_SOURCE_DIR}/tests/codegen/arch/${RUN_DIR}/run.sh
    4
    "10Kb"
  )
  add_test(CodegenTestsSemispace ${EXECUTABLE_OUTPUT_PATH}/codegen_tests)
endif()

unset(ARCH CACHE)
unset(GCTYPE CACHE)
