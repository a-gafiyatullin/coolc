if(ARCH STREQUAL "MIPS")
    set(ARCH_SRC
        arch/mips/asm/Assembler.cpp
        arch/mips/klass/KlassMips.cpp
        arch/mips/emitter/CodeGenMips.cpp
        arch/mips/emitter/data/DataMips.cpp
        arch/mips/runtime/RuntimeMips.cpp
        )
endif()

if(ARCH STREQUAL "LLVM")
    set(ARCH_SRC
        arch/llvm/klass/KlassLLVM.cpp
        arch/llvm/emitter/data/DataLLVM.cpp
        arch/llvm/emitter/CodeGenLLVM.cpp
        arch/llvm/runtime/RuntimeLLVM.cpp
        )
endif()

add_library(codegen STATIC
    ${ARCH_SRC}
    
    symnames/NameConstructor.cpp
    klass/Klass.cpp
    constants/Constants.cpp
    decls/Runtime.cpp
    )