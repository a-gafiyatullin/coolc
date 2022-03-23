#include "codegen/mips/SymbolTable.h"

using namespace codegen;

void SymbolTable::add_symbol(const std::string &name, const Symbol::SymbolType &type, const int &offset)
{
    CODEGEN_VERBOSE_ONLY(LOG("Add symbol " + name + " with type " + ((type == Symbol::FIELD) ? "FIELD" : "LOCAL") +
                             " and offset " + std::to_string(offset)));
    _symbols.back().emplace(name, Symbol(type, offset));
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
    CODEGEN_VERBOSE_ONLY(LOG("Can't find symbol " + symbol));
    SHOULD_NOT_REACH_HERE();
}