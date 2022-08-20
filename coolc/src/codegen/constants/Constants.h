#pragma once

#include <cstdint>

#define MARK_TYPE int32_t
#define TAG_TYPE uint32_t
#define SIZE_TYPE std::size_t
#define DISP_TAB_TYPE void *

#ifdef MIPS
#define WORD_SIZE 4
#endif // MIPS

#ifdef LLVM
#define WORD_SIZE 8
#endif // LLVM

#define TrueValue 1
#define FalseValue 0
#define DefaultValue 0

#define MarkWordDefaultValue -1
#define MarkWordSetValue 1
#define MarkWordUnsetValue 0

#define UnusedTag 0