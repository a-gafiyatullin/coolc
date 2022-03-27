#pragma once

#include <cstdint>

#ifdef MIPS
#define WORD_SIZE 4
#endif // MIPS

#ifdef LLVM
#define WORD_SIZE 8
#endif // LLVM

extern "C" int TrueVal;

extern "C" int FalseVal;