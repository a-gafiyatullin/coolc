# COOL PL PARSER
## Files:
- Parser source files are in <code>src/parser/</code> directory
- AST source files are in <code>src/ast/</code> directory
- Parser header files are in <code>include/parser/</code> directory
- AST header files are in <code>include/ast/</code> directory

Code is formatted using built-in VS Code clang-format with its default style, so there is no <code>.clang-format</code> file here.

## How to build:
- Build release version: <code>make parser</code>
- Build debug version with <b>ASan</b>: <code>make parser-asan</code>
- Build debug version with <b>UBSan</b>: <code>make parser-ubsan</code>

Executable <code>new-parser</code> is located in <code>bin/</code> directory.
## How to run tests:
- Run tests with release build: <code>make dotest</code>
- Run tests with debug version with <b>ASan</b>: <code>make dotest-asan</code>
- Run tests with debug version with <b>UBSan</b>: <code>make dotest-ubsan</code>

Tests are located in <code>tests/parser/</code>. There are tests from the original Edx course.

## P.S.
In <code>original/</code> directory you can find <code>cool.y</code> that is not a part of this project.