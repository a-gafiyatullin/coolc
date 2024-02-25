#pragma once

#include "codegen/constants/Constants.h"
#include "utils/Utils.h"
#include <unordered_map>

namespace codegen
{
/**
 * @brief Runtime declaration for rt-lib
 *
 */
template <class SymbolHandle> class Runtime
{
  protected:
    std::unordered_map<std::string, SymbolHandle *> _symbol_by_name;

  public:
    /**
     * @brief Get runtime symbol info by name
     *
     * @param name Runtime symbol name
     * @return Runtime symbol info
     */
    inline SymbolHandle *symbol(const std::string &name) const
    {
        return _symbol_by_name.find(name) != _symbol_by_name.end() ? _symbol_by_name.at(name) : nullptr;
    }

    /**
     * @brief Get runtime symbol name by id
     *
     * @param id Identifier
     * @return Runtime symbol name
     */
    virtual std::string symbol_name(const int &id) const = 0;

    /**
     * @brief Get runtime symbol info by id
     *
     * @param id Identifier
     * @return Runtime symbol info
     */
    inline SymbolHandle *symbol_by_id(const int &id) const
    {
        GUARANTEE_DEBUG(_symbol_by_name.find(symbol_name(id)) != _symbol_by_name.end());
        return symbol(symbol_name(id));
    }
};
}; // namespace codegen
