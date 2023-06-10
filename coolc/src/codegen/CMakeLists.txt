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

    arch/llvm/emitter/opt/nce/NCE.cpp
    arch/llvm/emitter/opt/dae/DAE.cpp
  )
endif()

if(ARCH STREQUAL "MYIR")
  set(ARCH_SRC
    arch/myir/klass/KlassMyIR.cpp
    arch/myir/emitter/data/DataMyIR.cpp
    arch/myir/emitter/CodeGenMyIR.cpp
    arch/myir/runtime/RuntimeMyIR.cpp
    arch/myir/ir/IR.cpp
    arch/myir/ir/Inst.cpp
    arch/myir/ir/CFG.cpp
    arch/myir/ir/SSA.cpp
    arch/myir/ir/Allocator.cpp
  )
endif()

add_library(codegen STATIC
  ${ARCH_SRC}

  symnames/NameConstructor.cpp
  klass/Klass.cpp
)