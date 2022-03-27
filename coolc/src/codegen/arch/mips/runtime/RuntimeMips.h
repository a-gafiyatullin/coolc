#pragma once

#include "codegen/arch/mips/asm/Assembler.h"
#include "codegen/decls/Runtime.h"

namespace codegen
{
/**
 * @brief Runtime declarations for SPIM
 *
 */
class RuntimeMips : public Runtime<const Label, Label>
{
  public:
    /**
     * @brief Runtime methods names
     *
     */
    static constexpr std::string_view EQUALITY_TEST = "equality_test";
    static constexpr std::string_view OBJECT_COPY = "Object.copy";
    static constexpr std::string_view CASE_ABORT = "_case_abort";
    static constexpr std::string_view CASE_ABORT2 = "_case_abort2";
    static constexpr std::string_view DISPATCH_ABORT = "_dispatch_abort";

    /**
     * @brief GC methods names
     *
     */
    static constexpr std::string_view GEN_GC_ASSIGN = "_GenGC_Assign";
    static constexpr std::string_view MEM_MGR_INIT = "_MemMgr_INITIALIZER";
    static constexpr std::string_view MEM_MGR_COLLECTOR = "_MemMgr_COLLECTOR";
    static constexpr std::string_view GEN_GC_INIT = "_GenGC_Init";
    static constexpr std::string_view MEM_MGR_TEST = "_MemMgr_TEST";
    static constexpr std::string_view GEN_GC_COLLECT = "_GenGC_Collect";

    /**
     * @brief Tag names
     *
     */
    static constexpr std::string_view INT_TAG_NAME = "_int_tag";
    static constexpr std::string_view BOOL_TAG_NAME = "_bool_tag";
    static constexpr std::string_view STRING_TAG_NAME = "_string_tag";

    /**
     * @brief Other runtime-related names
     *
     */
    static constexpr std::string_view HEAP_START = "heap_start";

  private:
    static constexpr std::string_view CLASS_OBJ_TAB = "class_objTab";
    static constexpr std::string_view CLASS_NAME_TAB = "class_nameTab";

    // runtime method labels
    const Label _equality_test;
    const Label _object_copy;
    const Label _case_abort;
    const Label _case_abort2;
    const Label _dispatch_abort;
    const Label _gen_gc_assign;

  public:
    RuntimeMips();

    /**
     * @brief Get runtime method info by name
     *
     * @param name Runtime method name
     * @return Runtime method info
     */
    inline const Label *method(const std::string_view &name)
    {
        return Runtime::method(static_cast<std::string>(name));
    }
};
}; // namespace codegen