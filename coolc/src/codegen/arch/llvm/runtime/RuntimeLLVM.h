#pragma once

#include "codegen/decls/Runtime.h"
#include <llvm/IR/Module.h>

namespace codegen
{
#define BITS_PER_BYTE 8

class RuntimeLLVM;

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

/**
 * @brief Information for one runtime method
 *
 */
struct RuntimeMethod
{
    llvm::Function *_func;

    /**
     * @brief Construct info for runtime method
     *
     * @param module LLVM Module to register method
     * @param name Method name
     * @param ret Return type
     * @param args Args types list
     * @param runtime Runtime info
     */
    RuntimeMethod(llvm::Module &module, const std::string &name, llvm::Type *ret,
                  const std::initializer_list<llvm::Type *> &args, RuntimeLLVM &runtime);
};

class RuntimeLLVM : public Runtime<RuntimeMethod>
{
    friend class RuntimeMethod;

  public:
    enum RuntimeLLVMSymbols
    {
        EQUALS,
        CASE_ABORT,
        CASE_ABORT_2,
        GC_ALLOC,
        DISPATCH_ABORT,

        INIT_RUNTIME,
        FINISH_RUNTIME,

        CLASS_NAME_TAB,
        CLASS_OBJ_TAB,

        INT_TAG_NAME,
        BOOL_TAG_NAME,
        STRING_TAG_NAME,

        RuntimeLLVMSymbolsSize
    };

  private:
    static const std::string SYMBOLS[RuntimeLLVMSymbolsSize];

    llvm::Type *const _int8_type;
    llvm::Type *const _int32_type;
    llvm::Type *const _int64_type;
    llvm::Type *const _default_int;
    llvm::Type *const _void_type;

    llvm::PointerType *const _stack_slot_type;
    llvm::PointerType *const _void_ptr_type;

    llvm::Type *_header_layout_types[HeaderLayoutElemets];

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

  public:
    /**
     * @brief Construct a new Runtime object
     *
     * @param module llvm::Module for function declarations
     * @param gc_strategy chosen GC Strategy
     */
    explicit RuntimeLLVM(llvm::Module &module);

    /**
     * @brief Get header layout element type
     *
     * @param elem Element
     * @return Type
     */
    inline llvm::Type *header_elem_type(const HeaderLayout &elem) const
    {
        return _header_layout_types[elem];
    }

    /**
     * @brief Get type for 32 bit int
     *
     * @return Type of 32 bit int
     */
    inline llvm::Type *int32_type() const
    {
        return _int32_type;
    }

    /**
     * @brief Get type for 64 bit int
     *
     * @return Type of 64 bit int
     */
    inline llvm::Type *int64_type() const
    {
        return _int64_type;
    }

    /**
     * @brief Get type for void
     *
     * @return Type of void
     */
    inline llvm::Type *void_type() const
    {
        return _void_type;
    }

    /**
     * @brief Get type for void pointer
     *
     * @return Type of void pointer
     */
    inline llvm::PointerType *void_ptr_type() const
    {
        return _void_ptr_type;
    }

    /**
     * @brief Get type for 8 bit int
     *
     * @return Type of 8 bit int
     */
    inline llvm::Type *int8_type() const
    {
        return _int8_type;
    }

    /**
     * @brief Get type of stack slot
     *
     * @return Type of stack slot
     */
    inline llvm::PointerType *stack_slot_type() const
    {
        return _stack_slot_type;
    }

    /**
     * @brief Get default int type
     *
     * @return Type for default int
     */
    inline llvm::Type *default_int() const
    {
        return _default_int;
    }

    std::string symbol_name(const int &id) const override
    {
        return SYMBOLS[id];
    }

    /**
     * @brief Get gc strategy name
     *
     * @return string for this strategy
     */
    inline std::string gc_strategy_name() const
    {
#ifdef LLVM_SHADOW_STACK
        return "shadow-stack";
#endif // LLVM_SHADOW_STACK
        return "statepoint-example";
    }
};
}; // namespace codegen