if(ARCH STREQUAL "MIPS")
    set(ARCH_SRC
        mips/Assembler.cpp
        mips/CodeGenMips.cpp
        mips/DataSection.cpp
        )
endif()

add_library(codegen STATIC
    ${ARCH_SRC}
    
    emitter/CodeGen.cpp
    namespace/NameSpace.cpp
    classes/Klass.cpp
    )