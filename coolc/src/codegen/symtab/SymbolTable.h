#pragma once

#include "utils/Utils.h"
#include <cassert>
#include <functional>
#include <unordered_map>
#include <vector>

namespace codegen
{
#ifdef MIPS
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
#endif // MIPS

/**
 * @brief Manage symbols in scopes
 *
 */
template <class T> class SymbolTable
{
  private:
    std::vector<std::unordered_map<std::string, T>> _symbols;   // class fields and locals offsets
    std::function<void(const std::string &, const T &)> _debug; // logging

  public:
    /**
     * @brief Construct a new SymbolTable with initial scope
     *
     */
#ifdef DEBUG
    SymbolTable(const std::function<void(const std::string &, const T &)> &debug_print)
        : _symbols(1), _debug(debug_print)
    {
    }
#else
    SymbolTable() : _symbols(1)
    {
    }
#endif // DEBUG

    /**
     * @brief Find symbol
     *
     * @param symbol Symbol name
     * @return Symbol object for this symbol
     */
    T &symbol(const std::string &symbol);

    /**
     * @brief Create Symbol
     *
     * @param name Symbol name
     * @param symbol Symbol object
     */
    void add_symbol(const std::string &name, const T &symbol);

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

template <class T> void SymbolTable<T>::add_symbol(const std::string &name, const T &symbol)
{
    CODEGEN_VERBOSE_ONLY(_debug(name, symbol));
    _symbols.back().emplace(name, symbol);
}

template <class T> T &SymbolTable<T>::symbol(const std::string &symbol)
{
    // search in reverse order
    for (int i = _symbols.size() - 1; i >= 0; i--)
    {
        const auto symbol_ptr = _symbols[i].find(symbol);
        if (symbol_ptr != _symbols[i].end())
        {
            return symbol_ptr->second;
        }
    }
    CODEGEN_VERBOSE_ONLY(LOG("Can't find symbol " + symbol));
    SHOULD_NOT_REACH_HERE();
}

}; // namespace codegen