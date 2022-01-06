#include "semant/Scope.h"

using namespace semant;

Scope::Scope(const std::shared_ptr<ast::Type> &self_type)
{
    _symbols.emplace_back();
    _symbols.back()[static_cast<std::string>(SELF)] = self_type;
}

Scope::AddResult Scope::add_if_can(const std::string &name, const std::shared_ptr<ast::Type> &type)
{
    SEMANT_RETURN_IF_FALSE(name != SELF, RESERVED);

    SEMANT_VERBOSE_ONLY(assert(_symbols.size() != 0));
    SEMANT_RETURN_IF_FALSE(_symbols.back().find(name) == _symbols.back().end(), REDEFINED);

    _symbols.back()[name] = type;
    return OK;
}

std::shared_ptr<ast::Type> Scope::find(const std::string &name, const int &scope_shift) const
{
    SEMANT_VERBOSE_ONLY(dump());

    for (auto class_scope = _symbols.rbegin() + scope_shift; class_scope != _symbols.rend(); class_scope++)
    {
        const auto symbol = class_scope->find(name);
        if (symbol != class_scope->end())
        {
            return symbol->second;
        }
    }

    return nullptr;
}

#ifdef DEBUG
void Scope::dump() const
{
    for (const auto &class_scope : _symbols)
    {
        std::cout << "--------------------------" << std::endl;
        for (const auto &symbol : class_scope)
        {
            std::cout << symbol.first << " = " << symbol.second->_string << std::endl;
        }
    }
}
#endif // DEBUG