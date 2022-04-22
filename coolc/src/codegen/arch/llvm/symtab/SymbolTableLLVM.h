#pragma once

#include "ast/AST.h"
#include "codegen/symtab/SymbolTable.h"
#include <llvm/IR/Value.h>

namespace codegen
{
struct Symbol
{
    enum SymbolType
    {
        FIELD,
        LOCAL
    };

    const SymbolType _type;
    const std::shared_ptr<ast::Type> _value_type;

    union {
        uint64_t _offset;
        llvm::Value *_ptr;
    } _value;

    /**
     * @brief Construct a new Symbol
     *
     * @param offset Offset from base
     */
    explicit Symbol(const uint64_t &offset, const std::shared_ptr<ast::Type> &type)
        : _type(SymbolType::FIELD), _value_type(type)
    {
        _value._offset = offset;
    }

    /**
     * @brief Construct a new Symbol
     *
     * @param val Value
     */
    explicit Symbol(llvm::Value *val, const std::shared_ptr<ast::Type> &type)
        : _type(SymbolType::LOCAL), _value_type(type)
    {
        _value._ptr = val;
    }

    operator std::string() const
    {
        // TODO: maybe add more info fo local
        return _type == SymbolType::FIELD ? "FIELD with index " + std::to_string(_value._offset) : "LOCAL";
    }
};
}; // namespace codegen