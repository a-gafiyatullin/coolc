# COOL PL LEXER
## Files:
- Source files are in <code>src/lexer/</code> directory
- Header files are in <code>include/lexer/</code> directory

Code is formatted using built-in VS Code clang-format with its default style, so there is no <code>.clang-format</code> file here.

## How to build:
- Build release version: <code>make lexer</code>
- Build debug version with <b>ASan</b>: <code>make lexer-asan</code>
- Build debug version with <b>UBSan</b>: <code>make lexer-ubsan</code>

Executable <code>new-lexer</code> is located in <code>bin/</code> directory.
## How to run tests:
- Run tests with release build: <code>make dotest</code>
- Run tests with debug version with <b>ASan</b>: <code>make dotest-asan</code>
- Run tests with debug version with <b>UBSan</b>: <code>make dotest-ubsan</code>

Tests are located in <code>tests/lexer/</code>. There are tests from the original Edx course.

## P.S.
In <code>original/</code> directory you can find <code>cool.flex</code> that is not a part of this project.