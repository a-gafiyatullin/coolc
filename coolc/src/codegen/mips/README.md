# COOL PL CODEGEN
## Files:
- Codegen source files are in <code>src/codegen/stack_machine/mips/spim/</code> directory
- Codegen header files are in <code>include/codegen/stack_machine/mips/spim/</code> directory

Code is formatted using built-in VS Code clang-format with its default style, so there is no <code>.clang-format</code> file here.

## How to build:
- Build release version: <code>make codegen</code>
- Build debug version with <b>ASan</b>: <code>make codegen-asan</code>
- Build debug version with <b>UBSan</b>: <code>make codegen-ubsan</code>

Executable <code>new-codegen</code> is located in <code>bin/</code> directory.
## How to run tests:
- Run tests with release build: <code>make dotest</code>
- Run tests with debug version with <b>ASan</b>: <code>make dotest-asan</code>
- Run tests with debug version with <b>UBSan</b>: <code>make dotest-ubsan</code>

Tests are located in <code>tests/codegen/</code>. There are tests from the original Edx course.
Test <code>nested-arith.cl</code> don't work with the reference <code>coolc</code>.