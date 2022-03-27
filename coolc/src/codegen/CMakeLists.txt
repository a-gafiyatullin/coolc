if(ARCH STREQUAL "MIPS")
    set(ARCH_SRC
        arch/mips/Assembler.cpp
        arch/mips/CodeGenMips.cpp
        arch/mips/DataSection.cpp
        )
endif()

add_library(codegen STATIC
    ${ARCH_SRC}
    
    emitter/CodeGen.cpp
    namespace/NameSpace.cpp
    classes/Klass.cpp
    )