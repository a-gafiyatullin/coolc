#pragma once

#include "utils/Utils.h"
#include <cassert>
#include <memory>
#include <numeric>
#include <string>
#include <unordered_map>
#include <vector>

namespace codegen
{
struct Symbol
{
    /**
     * @brief Different base for FIELD and LOCAL
     *
     */
    enum SymbolType
    {
        FIELD,
        LOCAL
    };

    const SymbolType _type;

    /**
     * @brief Offset from base
     *
     */
    const int _offset;

    /**
     * @brief Construct a new Symbol
     *
     * @param type Base type
     * @param offset Offset from base
     */
    Symbol(const SymbolType &type, const int &offset) : _type(type), _offset(offset)
    {
    }
};

/**
 * @brief Manage symbols in scopes
 *
 */
class SymbolTable
{
  private:
    std::vector<std::unordered_map<std::string, Symbol>> _symbols; // class fields and locals offsets

  public:
    /**
     * @brief Construct a new SymbolTable with initial scope
     *
     */
    SymbolTable() : _symbols(1)
    {
    }

    /**
     * @brief Find symbol
     *
     * @param symbol Symbol name
     * @return Symbol object for this symbol
     */
    Symbol &symbol(const std::string &symbol);

    /**
     * @brief Create Symbol
     *
     * @param name Symbol name
     * @param type Base type
     * @param offset Offset from base
     */
    void add_symbol(const std::string &name, const Symbol::SymbolType &type, const int &offset);

    /**
     * @brief Push new scope
     *
     */
    inline void push_scope()
    {
        _symbols.emplace_back();
    }

    /**
     * @brief Pop current scope
     *
     */
    inline void pop_scope()
    {
        GUARANTEE_DEBUG(!_symbols.empty());
        _symbols.pop_back();
    }
};

}; // namespace codegen