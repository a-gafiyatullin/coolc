ROOT= ../..

SRC	= Lexer.cpp Token.cpp

CXXINCLUDE = -I${ROOT}/include/
LEXER_MODE = -DLEXER_STANDALONE

BUILD_DIR = ${ROOT}/build
LEXER_BUILD_DIR = ${BUILD_DIR}/lexer
BIN_DIR = ${ROOT}/hwbin

CXX = g++
CXXFLAGS = -std=c++17 -O2 ${CXXINCLUDE} ${LEXER_MODE}

OBJS = ${SRC:%.cpp=${LEXER_BUILD_DIR}/%.o}

lexer: LEXER_MODE = -DLEXER_STANDALONE
lexer: compile-lexer

lexer-full-verbose: LEXER_MODE = -DLEXER_FULL_VERBOSE -DLEXER_STANDALONE
lexer-full-verbose: compile-lexer

compile-lexer: create_bin_dir create_build_dir ${OBJS}
	${CXX} ${CXXFLAGS} ${OBJS} -o ${BIN_DIR}/lexer

create_build_dir:
	-rm -rf ${LEXER_BUILD_DIR}
	mkdir -p ${LEXER_BUILD_DIR}

create_bin_dir:
	-mkdir -p ${BIN_DIR}

%.o: %.cpp
	$(CXX) ${CXXFLAGS} -c $<

$(LEXER_BUILD_DIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	-rm -f ${OBJS} ${BIN_DIR}/lexer