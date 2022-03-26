#include "codegen/symtab/SymbolTable.h"

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
}; // namespace codegen