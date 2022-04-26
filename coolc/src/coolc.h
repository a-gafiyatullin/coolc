#pragma once

#ifdef MIPS
#include "codegen/arch/mips/emitter/CodeGenMips.h"
#define CODEGEN codegen::CodeGenMips
#elif LLVM
#include "codegen/arch/llvm/emitter/CodeGenLLVM.h"
#define CODEGEN codegen::CodeGenLLVM
#else
#define CODEGEN UndefinedArchCodeGen
#endif // ARCH