#pragma once

#include "ast/AST.h"
#include "codegen/arch/myir/ir/Oper.hpp"
#include "codegen/symtab/SymbolTable.h"

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

    // for variables
    myir::Operand *_variable;

    // for fields
    const uint64_t _offset;

    /**
     * @brief Construct a new Symbol
     *
     * @param offset Offset from base
     * @param type Value type
     */
    Symbol(const uint64_t &offset, const std::shared_ptr<ast::Type> &type)
        : _type(SymbolType::FIELD), _value_type(type), _offset(offset), _variable(nullptr)
    {
    }

    /**
     * @brief Construct a new Symbol
     *
     * @param val Value
     * @param type Value type
     */
    Symbol(myir::Operand *var, const std::shared_ptr<ast::Type> &type)
        : _type(SymbolType::LOCAL), _value_type(type), _variable(var), _offset(-1)
    {
    }

    operator std::string() const
    {
        return _type == SymbolType::FIELD ? "FIELD with index " + std::to_string(_offset) : "LOCAL";
    }
};
}; // namespace codegen