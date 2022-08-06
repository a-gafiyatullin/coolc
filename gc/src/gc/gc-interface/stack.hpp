#pragma once

#include "object.hpp"
#include <vector>

namespace gc
{
template <class ObjectHeaderType> class GC;

/**
 * @brief StackRecord tracks root objects
 *
 */
template <class ObjectHeaderType> class StackRecord
{
  private:
    StackRecord *_parent;

    std::vector<address *> _objects;

    GC<ObjectHeaderType> *_gc;

  public:
    /**
     * @brief Construct a new StackRecord and adjust GC state
     *
     * @param gc Assosiated GC
     */
    StackRecord(GC<ObjectHeaderType> *gc);

    /**
     * @brief Construct a new StackRecord and adjust GC state
     *
     * @param gc Assosiated GC
     * @param parent Parent scope
     */
    StackRecord(StackRecord *parent);

    /**
     * @brief Destroy the StackRecord and adjust GC state
     *
     */
    ~StackRecord();

    /**
     * @brief Add a new root
     *
     */
    __attribute__((always_inline)) void reg_root(address *obj)
    {
        _objects.push_back(obj);
    }

    /**
     * @brief Get vector of the roots
     *
     * @return Vector of the roots
     */
    inline std::vector<address *> &roots_unsafe()
    {
        return _objects;
    }

    /**
     * @brief Parental Stack Record
     *
     * @return StackRecord
     */
    inline StackRecord *parent() const
    {
        return _parent;
    }
};
} // namespace gc