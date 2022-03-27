#pragma once

#include <unordered_map>

namespace codegen
{

enum RuntimeMethods
{
    EQUALS,
    GC_ALLOC,
    GC_ALLOC_BY_TAG,

    RuntimeMethodsSize
};

extern const char *const RuntimeMethodsNames[RuntimeMethodsSize];

enum RuntimeStructures
{
    CLASS_NAMES_TABLE,
    CLASS_OBJ_TABLE,

    RuntimeStructuresSize
};

extern const char *const RuntimeStructuresNames[RuntimeStructuresSize];

/**
 * @brief Runtime declaration for rt-lib
 *
 */
template <class MethodHandle, class LabelHandle> class Runtime
{
  protected:
    std::unordered_map<std::string, MethodHandle *> _method_by_name;

    // Runtime tables
    const LabelHandle _class_name_tab;
    const LabelHandle _class_obj_tab;

    Runtime(const LabelHandle &class_name_tab, const LabelHandle &class_obj_tab)
        : _class_name_tab(class_name_tab), _class_obj_tab(class_obj_tab)
    {
    }

  public:
    /**
     * @brief Get runtime method info by name
     *
     * @param name Runtime method name
     * @return Runtime method info
     */
    MethodHandle *method(const std::string &name) const
    {
        return _method_by_name.find(name) != _method_by_name.end() ? _method_by_name.at(name) : NULL;
    }

    /**
     * @brief Get class name table
     *
     * @return Class name table
     */
    const LabelHandle &class_name_tab() const
    {
        return _class_name_tab;
    }

    /**
     * @brief Get class object table
     *
     * @return Class object table
     */
    const LabelHandle &class_obj_tab() const
    {
        return _class_obj_tab;
    }
};
}; // namespace codegen