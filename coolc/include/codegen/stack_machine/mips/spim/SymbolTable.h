#pragma once

#include <unordered_map>
#include <cassert>
#include <numeric>
#include <vector>
#include "utils/Utils.h"

#ifdef CODEGEN_FULL_VERBOSE
#include "utils/logger/Logger.h"
#endif // CODEGEN_FULL_VERBOSE

namespace codegen
{
    struct Symbol
    {
        // we should use different base for FIELD and LOCAL
        enum SYMBOL_TYPE
        {
            FIELD,
            LOCAL
        };

        const SYMBOL_TYPE _type;
        const int _offset;

        Symbol(const SYMBOL_TYPE &type, const int &offset) : _type(type), _offset(offset) {}
    };

    // manage symbols in scopes
    class SymbolTable
    {
    private:
        std::vector<std::unordered_map<std::string, Symbol>> _symbols; // class fields and locals offsets

        CODEGEN_FULL_VERBOSE_ONLY(Logger _logger;);

    public:
        SymbolTable() : _symbols(1) {}

        // find symbol
        Symbol &get_symbol(const std::string &symbol);

        // create Symbol with name "name" and offset [offset]
        void add_symbol(const std::string &name, const Symbol::SYMBOL_TYPE &type, const int &offset);
        // number of symbols
        int count() const;

        // scope managing
        inline void push_scope() { _symbols.emplace_back(); }
        inline void pop_scope() { _symbols.pop_back(); }

#ifdef CODEGEN_FULL_VERBOSE
        inline void set_parent_logger(Logger *logger) { _logger.set_parent_logger(logger); }
#endif // CODEGEN_FULL_VERBOSE
    };
};