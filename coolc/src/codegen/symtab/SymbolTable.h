#pragma once

#include "utils/Utils.h"
#include <functional>
#include <unordered_map>

namespace codegen
{
/**
 * @brief Manage symbols in scopes
 *
 */
template <class T> class SymbolTable
{
  private:
    std::vector<std::unordered_map<std::string, T>> _symbols; // class fields and locals offsets

#ifdef DEBUG
    std::function<void(const std::string &, const T &)> _debug; // logging
#endif                                                          // DEBUG
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

#ifdef DEBUG
    void set_printer(const std::function<void(const std::string &, const T &)> &debug_print)
    {
        _debug = debug_print;
    }
#endif // DEBUG
};

template <class T> void SymbolTable<T>::add_symbol(const std::string &name, const T &symbol)
{
    CODEGEN_VERBOSE_ONLY(_debug(name, symbol));
    _symbols.back().emplace(name, symbol);
}

template <class T> T &SymbolTable<T>::symbol(const std::string &symbol)
{
    // search in reverse order
    for (auto i = _symbols.size() - 1; i >= 0; i--)
    {
        const auto symbol_ptr = _symbols[i].find(symbol);
        if (symbol_ptr != _symbols[i].end())
        {
            return symbol_ptr->second;
        }
    }
    CODEGEN_VERBOSE_ONLY(LOG("Can't find symbol \"" + symbol + "\""));
    SHOULD_NOT_REACH_HERE();
}
}; // namespace codegen