CODEGEN_ROOT = ../../..
ROOT= ${CODEGEN_ROOT}/../..

SRC	= Assembler.cpp CodeGen.cpp SymbolTable.cpp DataSection.cpp ${CODEGEN_ROOT}/../utils/logger/Logger.cpp ${CODEGEN_ROOT}/../coolc.cpp

CXXINCLUDE = -I${ROOT}/include/
CODEGEN_MODE = 

BUILD_DIR = ${ROOT}/build
CODEGEN_BUILD_DIR = ${BUILD_DIR}/codegen/stack_machine/mips/spim
LOGGER_BUILD_DIR = ${BUILD_DIR}/utils/logger
BIN_DIR = ${ROOT}/bin
TEST_DIR = ${ROOT}/tests/codegen
LIB_DIR = ${ROOT}/lib

CXX = clang++-10
BASIC_CXXFLAGS = -std=c++20 ${CXXINCLUDE} ${CODEGEN_MODE} -Wno-switch -Wno-return-type
CXXFLAGS = ${BASIC_CXXFLAGS} -O2
DEBUG_CXXFLAGS = ${BASIC_CXXFLAGS} -g -O0
LDFLAGS = 

OBJS = ${SRC:%.cpp=${CODEGEN_BUILD_DIR}/%.o}

codegen: .codegen

codegen-full-verbose: CODEGEN_MODE = -DCODEGEN_FULL_VERBOSE
codegen-full-verbose: .codegen

.lexer.a:
	cd ${ROOT}/src/lexer/ && $(MAKE) lexer-static-release

.parser.a:
	cd ${ROOT}/src/parser/ && $(MAKE) parser-static-verbose

.semant.a:
	cd ${ROOT}/src/semant/ && $(MAKE) semant-static-release

.codegen: .lexer.a .parser.a .semant.a .create_build_dir ${OBJS}
	${CXX} ${LDFLAGS} ${OBJS} ${LIB_DIR}/semant.a ${LIB_DIR}/lexer.a ${LIB_DIR}/parser.a -o ${BIN_DIR}/new-codegen

.create_build_dir:
	rm -rf ${CODEGEN_BUILD_DIR}
	rm -rf ${LOGGER_BUILD_DIR}
	rm -rf ${BUILD_DIR}/coolc.o
	mkdir -p ${CODEGEN_BUILD_DIR}
	mkdir -p ${LOGGER_BUILD_DIR}

codegen-asan: CXXFLAGS = ${DEBUG_CXXFLAGS} -fsanitize=address -fno-omit-frame-pointer
codegen-asan: LDFLAGS = -fsanitize=address -g
codegen-asan: .codegen

codegen-ubsan: CXXFLAGS = ${DEBUG_CXXFLAGS} -fsanitize=undefined -fno-omit-frame-pointer
codegen-ubsan: LDFLAGS = -fsanitize=undefined -g
codegen-ubsan: .codegen

codegen-debug: CXXFLAGS = ${DEBUG_CXXFLAGS}
codegen-debug: LDFLAGS += -g
codegen-debug: .codegen

dotest:
	python3 ${TEST_DIR}/test.py -build

dotest-asan:
	python3 ${TEST_DIR}/test.py -asan

dotest-ubsan:
	python3 ${TEST_DIR}/test.py -ubsan

${CODEGEN_BUILD_DIR}/%.o: %.cpp
	${CXX} ${CXXFLAGS} -DCOOLC -c $< -o $@

clean:
	-rm -f ${OBJS} ${BIN_DIR}/new-codegen