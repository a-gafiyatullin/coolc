#pragma once

#include "codegen/arch/mips/asm/Assembler.h"
#include "codegen/decls/Runtime.h"

namespace codegen
{
/**
 * @brief Runtime declarations for SPIM
 *
 */
class RuntimeMips : public Runtime<const Label>
{
  public:
    enum RuntimeMipsSymbols
    {
        EQUALITY_TEST,
        OBJECT_COPY,
        CASE_ABORT,
        CASE_ABORT2,
        DISPATCH_ABORT,

        GEN_GC_ASSIGN,
        MEM_MGR_INIT,
        MEM_MGR_COLLECTOR,
        GEN_GC_INIT,
        MEM_MGR_TEST,
        GEN_GC_COLLECT,

        INT_TAG_NAME,
        BOOL_TAG_NAME,
        STRING_TAG_NAME,

        HEAP_START,

        CLASS_OBJ_TAB,
        CLASS_NAME_TAB,

        RuntimeMipsSymbolsSize
    };

  private:
    static const std::string SYMBOLS[RuntimeMipsSymbolsSize];

    // Runtime tables
    const Label _class_name_tab;
    const Label _class_obj_tab;

    // runtime method labels
    const Label _equality_test;
    const Label _object_copy;
    const Label _case_abort;
    const Label _case_abort2;
    const Label _dispatch_abort;
    const Label _gen_gc_assign;

  public:
    RuntimeMips();

    std::string symbol_name(const int &id) const override
    {
        return SYMBOLS[id];
    }
};
}; // namespace codegen