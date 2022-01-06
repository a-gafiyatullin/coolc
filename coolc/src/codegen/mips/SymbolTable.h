#pragma once

#include "utils/Utils.h"
#include <cassert>
#include <memory>
#include <numeric>
#include <unordered_map>
#include <vector>

#ifdef DEBUG
#include "utils/logger/Logger.h"
#endif // DEBUG

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

#ifdef DEBUG
    std::shared_ptr<Logger> _logger;
#endif // DEBUG

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
    Symbol &get_symbol(const std::string &symbol);

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
        _symbols.pop_back();
    }

    /**
     * @brief Count number of symbol that are in the table
     *
     * @return Number of symbols
     */
    int count() const;

#ifdef DEBUG
    /**
     * @brief Set the parent logger
     *
     * @param logger Parent logger
     */
    inline void set_parent_logger(const std::shared_ptr<Logger> logger)
    {
        _logger->set_parent_logger(logger);
    }
#endif // DEBUG
};

}; // namespace codegen