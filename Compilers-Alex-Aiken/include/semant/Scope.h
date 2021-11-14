#pragma once

#include "ast/AST.h"
#include "utils/Utils.h"
#include <unordered_map>

#ifdef SEMANT_FULL_VERBOSE
#include <cassert>
#endif // SEMANT_FULL_VERBOSE

#define SEMANT_RETURN_IF_FALSE(cond, retval) \
    if (!(cond))                             \
    {                                        \
        return retval;                       \
    }

namespace semant
{
    class Scope
    {
    private:
        std::vector<std::unordered_map<std::string, std::shared_ptr<ast::Type>>> _symbols; // vectors of maps for convenient way to model class scopes
        static constexpr std::string_view _self = "self";                                  // cannot be names of symbols

    public:
        explicit Scope(const std::shared_ptr<ast::Type> &self_type); // create Scope with initial scope with self object

        enum ADD_RESULT
        {
            OK,
            RESERVED,
            REDEFINED
        };

        inline void push_scope() { _symbols.emplace_back(); }
        inline void pop_scope() { _symbols.pop_back(); }

        ADD_RESULT add_if_can(const std::string &name, const std::shared_ptr<ast::Type> &type);
        inline static bool can_assign(const std::string &name) { return name != _self; }

        std::shared_ptr<ast::Type> find(const std::string &name, const int &scope_shift = 0) const;

#ifdef SEMANT_FULL_VERBOSE
        void dump() const;
#endif // SEMANT_FULL_VERBOSE
    };
}