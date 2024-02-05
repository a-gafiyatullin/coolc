#pragma once

#include "ast/AST.h"
#include "decls/Decls.h"
#include "utils/Utils.h"
#include <unordered_map>

#define SEMANT_RETURN_IF_FALSE(cond, retval)                                                                           \
    if (!(cond))                                                                                                       \
    {                                                                                                                  \
        return retval;                                                                                                 \
    }

namespace semant
{

class Scope
{
  private:
    std::vector<std::unordered_map<std::string, std::shared_ptr<ast::Type>>>
        _symbols; // vectors of maps for convenient way to model class scopes

  public:
    /**
     * @brief Construct a new Scope
     *
     * @param self_type Current class of this scope
     */
    explicit Scope(const std::shared_ptr<ast::Type> &self_type); // create Scope with initial scope with self object

    /**
     * @brief Results of adding new elements to scope
     *
     */
    enum AddResult
    {
        OK,
        RESERVED,
        REDEFINED
    };

    /**
     * @brief Start new scope
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
        GUARANTEE_DEBUG(_symbols.size() != 0);
        _symbols.pop_back();
    }

    /**
     * @brief Add new element to the current scoope
     *
     * @param name Element name
     * @param type Element type
     * @return Status
     */
    AddResult add_if_can(const std::string &name, const std::shared_ptr<ast::Type> &type);

    /**
     * @brief Check if assignment for given element is prohibited
     *
     * @param name Element name
     * @return True if assignment is allowed
     */
    inline static bool can_assign(const std::string &name)
    {
        return name != SelfObject;
    }

    /**
     * @brief Find element in the scope
     *
     * @param name Element for lookup
     * @param scope_shift Start lookup from previos scope_shift scopes
     * @return Type of the element
     */
    std::shared_ptr<ast::Type> find(const std::string &name, const int &scope_shift = 0) const;

#ifdef DEBUG
    /**
     * @brief Dump containings of the scope
     *
     */
    void dump() const;
#endif // DEBUG
};

} // namespace semant