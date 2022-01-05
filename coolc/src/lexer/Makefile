ROOT= ../..

SRC	= Lexer.cpp Token.cpp ../utils/Utils.cpp ../utils/logger/Logger.cpp ../coolc.cpp

CXXINCLUDE = -I${ROOT}/src/
LEXER_MODE = -DLEXER_STANDALONE

BUILD_DIR = ${ROOT}/build
LEXER_BUILD_DIR = ${BUILD_DIR}/lexer
UTILS_BUILD_DIR = ${BUILD_DIR}/utils
BIN_DIR = ${ROOT}/bin
TEST_DIR = ${ROOT}/tests/lexer
LIB_DIR = ${ROOT}/lib

CXX = clang++-10
BASIC_CXXFLAGS = -std=c++20 ${CXXINCLUDE} ${LEXER_MODE}
CXXFLAGS = ${BASIC_CXXFLAGS} -O2
DEBUG_CXXFLAGS = ${BASIC_CXXFLAGS} -g -O0
LDFLAGS = 

OBJS = ${SRC:%.cpp=${LEXER_BUILD_DIR}/%.o}

lexer: LEXER_MODE = -DLEXER_STANDALONE
lexer: .compile-lexer

lexer-full-verbose: LEXER_MODE = -DLEXER_FULL_VERBOSE -DLEXER_STANDALONE
lexer-full-verbose: .compile-lexer

.compile-lexer: .create_build_dir ${OBJS}
	${CXX} ${LDFLAGS} ${OBJS} -o ${BIN_DIR}/new-lexer

.create_build_dir:
	rm -rf ${LEXER_BUILD_DIR}
	rm -rf ${UTILS_BUILD_DIR}
	rm -rf ${BUILD_DIR}/coolc.o
	mkdir -p ${LEXER_BUILD_DIR}
	mkdir -p ${UTILS_BUILD_DIR}
	mkdir -p ${UTILS_BUILD_DIR}/logger

.create_lib_dir:
	-mkdir -p ${ROOT}/lib

lexer-asan: CXXFLAGS = ${DEBUG_CXXFLAGS} -fsanitize=address -fno-omit-frame-pointer
lexer-asan: LDFLAGS = -fsanitize=address -g
lexer-asan: lexer

lexer-ubsan: CXXFLAGS = ${DEBUG_CXXFLAGS} -fsanitize=undefined -fno-omit-frame-pointer
lexer-ubsan: LDFLAGS = -fsanitize=undefined -g
lexer-ubsan: lexer

lexer-static-release: LEXER_MODE =
lexer-static-release: .create_lib_dir .create_build_dir ${OBJS}
	ar rvs ${LIB_DIR}/lexer.a ${OBJS}

lexer-static-debug: CXXFLAGS = ${DEBUG_CXXFLAGS}
lexer-static-debug: LDFLAGS += -g
lexer-static-debug: lexer-static-release

dotest:
	python3 ${TEST_DIR}/test.py -build

dotest-asan:
	python3 ${TEST_DIR}/test.py -asan

dotest-ubsan:
	python3 ${TEST_DIR}/test.py -ubsan

${LEXER_BUILD_DIR}/%.o: %.cpp
	${CXX} ${CXXFLAGS} -c $< -o $@

clean:
	-rm -f ${OBJS} ${BIN_DIR}/new-lexer ${LIB_DIR}/lexer.a