# COOL PL SEMANT
## Files:
- Semant source files are in <code>src/semant/</code> directory
- AST source files are in <code>src/ast/</code> directory
- Semant header files are in <code>include/semant/</code> directory
- AST header files are in <code>include/ast/</code> directory

Code is formatted using built-in VS Code clang-format with its default style, so there is no <code>.clang-format</code> file here.

## How to build:
- Build release version: <code>make semant</code>
- Build debug version with <b>ASan</b>: <code>make semant-asan</code>
- Build debug version with <b>UBSan</b>: <code>make semant-ubsan</code>

Executable <code>new-semant</code> is located in <code>bin/</code> directory.
## How to run tests:
- Run tests with release build: <code>make dotest</code>
- Run tests with debug version with <b>ASan</b>: <code>make dotest-asan</code>
- Run tests with debug version with <b>UBSan</b>: <code>make dotest-ubsan</code>

Tests are located in <code>tests/semant/</code>. There are tests from the original Edx course.