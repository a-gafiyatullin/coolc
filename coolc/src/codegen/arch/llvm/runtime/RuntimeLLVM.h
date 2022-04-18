#pragma once

#include "codegen/decls/Runtime.h"
#include <llvm/IR/Module.h>

namespace codegen
{
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
    MarkSize = sizeof(MARK_TYPE) * WORD_SIZE,
    TagSize = sizeof(TAG_TYPE) * WORD_SIZE,
    SizeSize = sizeof(SIZE_TYPE) * WORD_SIZE,
    DispatchTableSize = sizeof(DISP_TAB_TYPE) * WORD_SIZE,

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
        OBJECT_ABORT,
        OBJECT_TYPE_NAME,
        OBJECT_COPY,

        STRING_LENGTH,
        STRING_CONCAT,
        STRING_SUBSTR,

        IO_OUT_STRING,
        IO_OUT_INT,
        IO_IN_STRING,
        IO_IN_INT,

        EQUALS,
        GC_ALLOC,

        CLASS_NAME_TAB,
        CLASS_OBJ_TAB,

        RuntimeLLVMSymbolsSize
    };

  private:
    static const std::string SYMBOLS[RuntimeLLVMSymbolsSize];

    llvm::Type *const _int8_type;
    llvm::Type *const _int32_type;
    llvm::Type *const _int64_type;
    llvm::Type *const _void_type;

    llvm::Type *_header_layout_types[HeaderLayoutElemets];

    const RuntimeMethod _equals;

    // Object class methods
    const RuntimeMethod _object_abort;
    const RuntimeMethod _object_type_name;
    const RuntimeMethod _object_copy;

    // String methods
    const RuntimeMethod _string_length;
    const RuntimeMethod _string_concat;
    const RuntimeMethod _string_substr;

    // IO methods
    const RuntimeMethod _io_out_string;
    const RuntimeMethod _io_out_int;
    const RuntimeMethod _io_in_string;
    const RuntimeMethod _io_in_int;

    // GC
    const RuntimeMethod _gc_alloc;

  public:
    /**
     * @brief Construct a new Runtime object
     *
     * @param module llvm::Module for function declarations
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
     * @brief Get type for 8 bit int
     *
     * @return Type of 8 bit int
     */
    inline llvm::Type *int8_type() const
    {
        return _int8_type;
    }

    std::string symbol_name(const int &id) const override
    {
        return SYMBOLS[id];
    }
};
}; // namespace codegen