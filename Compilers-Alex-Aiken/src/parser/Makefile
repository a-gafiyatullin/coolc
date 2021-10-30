ROOT= ../..

SRC	= Parser.cpp ../ast/AST.cpp ../utils/Utils.cpp

CXXINCLUDE = -I${ROOT}/include/
PARSER_MODE = -DPARSER_STANDALONE

BUILD_DIR = ${ROOT}/build
PARSER_BUILD_DIR = ${BUILD_DIR}/parser
UTILS_BUILD_DIR = ${BUILD_DIR}/utils
AST_BUILD_DIR = ${BUILD_DIR}/ast
BIN_DIR = ${ROOT}/bin
TEST_DIR = ${ROOT}/tests/parser
LIB_DIR = ${ROOT}/lib

CXX = clang++-10
BASIC_CXXFLAGS = -std=c++20 ${CXXINCLUDE} ${PARSER_MODE} -Wno-switch -Wno-return-type
CXXFLAGS = ${BASIC_CXXFLAGS} -O2
DEBUG_CXXFLAGS = ${BASIC_CXXFLAGS} -g -O0
LDFLAGS = 

OBJS = ${SRC:%.cpp=${PARSER_BUILD_DIR}/%.o}

parser: PARSER_MODE = -DPARSER_STANDALONE
parser: compile-parser

parser-full-verbose: PARSER_MODE = -DPARSER_FULL_VERBOSE -DPARSER_STANDALONE
parser-full-verbose: compile-parser

lexer.a:
	cd ../lexer/ && $(MAKE) lexer-static

compile-parser: lexer.a create_build_dir ${OBJS}
	${CXX} ${LDFLAGS} ${OBJS} ${LIB_DIR}/lexer.a -o ${BIN_DIR}/new-parser

create_build_dir:
	rm -rf ${PARSER_BUILD_DIR}
	rm -rf ${UTILS_BUILD_DIR}
	rm -rf ${AST_BUILD_DIR}
	mkdir -p ${PARSER_BUILD_DIR}
	mkdir -p ${UTILS_BUILD_DIR}
	mkdir -p ${AST_BUILD_DIR}

create_lib_dir:
	-mkdir -p ${ROOT}/lib

parser-asan: CXXFLAGS = ${DEBUG_CXXFLAGS} -fsanitize=address -fno-omit-frame-pointer
parser-asan: LDFLAGS = -fsanitize=address -g
parser-asan: parser

parser-ubsan: CXXFLAGS = ${DEBUG_CXXFLAGS} -fsanitize=undefined -fno-omit-frame-pointer
parser-ubsan: LDFLAGS = -fsanitize=undefined -g
parser-ubsan: parser

parser-shared-release: PARSER_MODE =
parser-shared-release: CXXFLAGS += -fPIC
parser-shared-release: LDFLAGS = -shared -fPIC
parser-shared-release: create_lib_dir create_build_dir ${OBJS}
	${CXX} ${LDFLAGS} ${OBJS} -o ${LIB_DIR}/parser.so

parser-shared-debug: CXXFLAGS = ${DEBUG_CXXFLAGS}
parser-shared-debug: LDFLAGS += -g
parser-shared-debug: parser-shared-release

dotest:
	python3 ${TEST_DIR}/test.py -build

dotest-asan:
	python3 ${TEST_DIR}/test.py -asan

dotest-ubsan:
	python3 ${TEST_DIR}/test.py -ubsan

${PARSER_BUILD_DIR}/%.o: %.cpp
	${CXX} ${CXXFLAGS} -c $< -o $@

clean:
	-rm -f ${OBJS} ${BIN_DIR}/new-parser ${LIB_DIR}/parser.so