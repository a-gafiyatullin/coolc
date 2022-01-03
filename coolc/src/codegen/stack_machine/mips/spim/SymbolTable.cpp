#include "codegen/stack_machine/mips/spim/SymbolTable.h"

using namespace codegen;

void SymbolTable::add_symbol(const std::string &name, const Symbol::SYMBOL_TYPE &type, const int &offset)
{
    CODEGEN_FULL_VERBOSE_ONLY(_logger.log("Add symbol " + name + " with type " + ((type == Symbol::FIELD) ? "FIELD" : "LOCAL") +
                                          " and offset " + std::to_string(offset)));
    _symbols.back().insert(std::make_pair(name, Symbol(type, offset)));
}

Symbol &SymbolTable::get_symbol(const std::string &symbol)
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
    CODEGEN_FULL_VERBOSE_ONLY(_logger.log("Can't find symbol " + symbol));
    assert(false && "SymbolTable::get_symbol: no such symbol!"); // bad situation
}

int SymbolTable::count() const
{
    return std::accumulate(_symbols.begin(), _symbols.end(), 0,
                           [](int acc, const auto &s)
                           { return acc + s.size(); });
}