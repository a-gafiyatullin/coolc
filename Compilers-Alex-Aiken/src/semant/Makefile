ROOT= ../..

SRC	= Semant.cpp Scope.cpp ../utils/logger/Logger.cpp ../coolc.cpp

CXXINCLUDE = -I${ROOT}/include/
SEMANT_MODE = -DSEMANT_STANDALONE

BUILD_DIR = ${ROOT}/build
SEMANT_BUILD_DIR = ${BUILD_DIR}/semant
LOGGER_BUILD_DIR = ${BUILD_DIR}/utils/logger
BIN_DIR = ${ROOT}/bin
TEST_DIR = ${ROOT}/tests/semant
LIB_DIR = ${ROOT}/lib

CXX = clang++-10
BASIC_CXXFLAGS = -std=c++20 ${CXXINCLUDE} ${SEMANT_MODE} -Wno-switch -Wno-return-type
CXXFLAGS = ${BASIC_CXXFLAGS} -O2
DEBUG_CXXFLAGS = ${BASIC_CXXFLAGS} -g -O0
LDFLAGS = 

OBJS = ${SRC:%.cpp=${SEMANT_BUILD_DIR}/%.o}

semant: SEMANT_MODE = -DSEMANT_STANDALONE
semant: .compile-semant

semant-full-verbose: SEMANT_MODE = -DSEMANT_FULL_VERBOSE -DSEMANT_STANDALONE
semant-full-verbose: .compile-semant

.lexer.a:
	cd ../lexer/ && $(MAKE) lexer-static-release

.parser.a:
	cd ../parser/ && $(MAKE) parser-static-verbose

.compile-semant: .lexer.a .parser.a .create_build_dir ${OBJS}
	${CXX} ${LDFLAGS} ${OBJS} ${LIB_DIR}/lexer.a ${LIB_DIR}/parser.a -o ${BIN_DIR}/new-semant

.create_build_dir:
	rm -rf ${SEMANT_BUILD_DIR}
	rm -rf ${LOGGER_BUILD_DIR}
	rm -rf ${BUILD_DIR}/coolc.o
	mkdir -p ${SEMANT_BUILD_DIR}
	mkdir -p ${LOGGER_BUILD_DIR}

.create_lib_dir:
	-mkdir -p ${ROOT}/lib

semant-asan: CXXFLAGS = ${DEBUG_CXXFLAGS} -fsanitize=address -fno-omit-frame-pointer
semant-asan: LDFLAGS = -fsanitize=address -g
semant-asan: semant

semant-ubsan: CXXFLAGS = ${DEBUG_CXXFLAGS} -fsanitize=undefined -fno-omit-frame-pointer
semant-ubsan: LDFLAGS = -fsanitize=undefined -g
semant-ubsan: semant

semant-static-release: SEMANT_MODE =
semant-static-release: .create_lib_dir .create_build_dir ${OBJS}
	ar rvs ${LIB_DIR}/semant.a ${OBJS}

semant-static-debug: CXXFLAGS = ${DEBUG_CXXFLAGS}
semant-static-debug: LDFLAGS += -g
semant-static-debug: semant-static-release

dotest:
	python3 ${TEST_DIR}/test.py -build

dotest-asan:
	python3 ${TEST_DIR}/test.py -asan

dotest-ubsan:
	python3 ${TEST_DIR}/test.py -ubsan

${SEMANT_BUILD_DIR}/%.o: %.cpp
	${CXX} ${CXXFLAGS} -c $< -o $@

clean:
	-rm -f ${OBJS} ${BIN_DIR}/new-semant ${LIB_DIR}/semant.a