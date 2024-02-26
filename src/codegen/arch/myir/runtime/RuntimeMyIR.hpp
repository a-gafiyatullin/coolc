#pragma once

#include "codegen/arch/myir/ir/IR.hpp"
#include "codegen/decls/Runtime.h"

namespace codegen
{
class RuntimeMyIR;

enum HeaderLayout
{
    Mark,
    Tag,
    Size,
    DispatchTable,

    HeaderLayoutElemets
};

enum HeaderLayoutSizes
{
    MarkSize = sizeof(MARK_TYPE),
    TagSize = sizeof(TAG_TYPE),
    SizeSize = sizeof(SIZE_TYPE),
    DispatchTableSize = sizeof(DISP_TAB_TYPE),

    HeaderSize = MarkSize + TagSize + SizeSize + DispatchTableSize
};

enum HeaderLayoutOffsets
{
    MarkOffset = 0,
    TagOffset = MarkSize,
    SizeOffset = TagOffset + TagSize,
    DispatchTableOffset = SizeOffset + SizeSize,
    FieldOffset = DispatchTableOffset + DispatchTableSize
};

/**
 * @brief Information for one runtime method
 *
 */
struct RuntimeMethod
{
    myir::Function *_func;

    /**
     * @brief Construct info for runtime method
     *
     * @param module Module to register method
     * @param name Method name
     * @param ret Return type
     * @param args Args types list
     * @param need_gc Can cause gc
     * @param runtime Runtime info
     */
    RuntimeMethod(myir::Module &module, const std::string &name, myir::OperandType ret,
                  const std::vector<myir::Variable *> &args, bool need_gc, RuntimeMyIR &runtime);
};

class RuntimeMyIR : public Runtime<RuntimeMethod>
{
    friend class RuntimeMethod;

  public:
    enum RuntimeMyIRSymbols
    {
        EQUALS,
        CASE_ABORT,
        CASE_ABORT_2,
        GC_ALLOC,
        DISPATCH_ABORT,

        INIT_RUNTIME,
        FINISH_RUNTIME,

#ifdef DEBUG
        VERIFY_OOP,
#endif // DEBUG

        CLASS_NAME_TAB,
        CLASS_OBJ_TAB,

        INT_TAG_NAME,
        BOOL_TAG_NAME,
        STRING_TAG_NAME,

        STACK_POINTER,
        FRAME_POINTER,

        RuntimeMyIRSymbolsSize
    };

  private:
    static const std::string SYMBOLS[RuntimeMyIRSymbolsSize];

    myir::OperandType _header_layout_types[HeaderLayoutElemets];

    const RuntimeMethod _equals;

    // Exceptional situations
    const RuntimeMethod _case_abort;
    const RuntimeMethod _case_abort_2;
    const RuntimeMethod _dispatch_abort;

    // GC
    const RuntimeMethod _gc_alloc;

    // runtime init
    const RuntimeMethod _init_runtime;
    const RuntimeMethod _finish_runtime;

#ifdef DEBUG
    const RuntimeMethod _verify_oop;
#endif // DEBUG

    myir::GlobalVariable *_stack_pointer;
    myir::GlobalVariable *_frame_pointer;

  public:
    /**
     * @brief Construct a new Runtime object
     *
     * @param module Module for function declarations
     */
    explicit RuntimeMyIR(myir::Module &module);

    /**
     * @brief Get header layout element type
     *
     * @param elem Element
     * @return Type
     */
    inline myir::OperandType header_elem_type(const HeaderLayout &elem) const { return _header_layout_types[elem]; }

    std::string symbol_name(const int &id) const override { return SYMBOLS[id]; }

    /**
     * @brief Get thread local variable to store stack pointer
     *
     * @return myir::Operand*
     */
    myir::Operand *stack_pointer() const { return _stack_pointer; }

    /**
     * @brief Get thread local variable to store frame pointer
     *
     * @return myir::Operand*
     */
    myir::Operand *frame_pointer() const { return _frame_pointer; }
};
}; // namespace codegen
