#pragma once

#include "codegen/klass/Klass.h"
#include <fstream>

namespace codegen
{
/**
 * @brief Maintain constants
 *
 */
template <class Value, class ClassDesc = Value, class DispTableDesc = Value> class Data
{
  protected:
    const std::shared_ptr<KlassBuilder> _builder;

    // constants
    std::unordered_map<bool, std::remove_cvref_t<Value>> _bool_constants;
    std::unordered_map<std::string, std::remove_cvref_t<Value>> _string_constants;
    std::unordered_map<int64_t, std::remove_cvref_t<Value>> _int_constants;

    std::unordered_map<std::string, std::remove_cvref_t<ClassDesc>> _classes;
    std::unordered_map<std::string, std::remove_cvref_t<DispTableDesc>> _dispatch_tables;

    virtual void string_const_inner(const std::string &str) = 0;
    virtual void bool_const_inner(const bool &value) = 0;
    virtual void int_const_inner(const int64_t &value) = 0;
    virtual void class_struct_inner(const std::shared_ptr<Klass> &klass) = 0;
    virtual void class_disp_tab_inner(const std::shared_ptr<Klass> &klass) = 0;

    virtual void gen_class_obj_tab() = 0;
    virtual void gen_class_name_tab() = 0;

    virtual void emit_inner(const std::string &out_file) = 0;

  public:
    /**
     * @brief Construct a new DataMips object
     *
     * @param builder KlassBuilder
     */
    explicit Data(const std::shared_ptr<KlassBuilder> &builder);

    /**
     * @brief Declare string constant
     *
     * @param str String value
     * @return Value associated with a given string
     */
    Value string_const(const std::string &str);

    /**
     * @brief Declare boolean constant
     *
     * @param value Boolean
     * @return Value associated with a given bolean
     */
    Value bool_const(const bool &value);

    /**
     * @brief Declare integer constant
     *
     * @param value Integer value
     * @return Value associated with a given integer
     */
    Value int_const(const int64_t &value);

    /**
     * @brief Get low-level class handle
     *
     * @param klass High-level class handle
     * @return Class handle
     */
    ClassDesc class_struct(const std::shared_ptr<Klass> &klass);

    /**
     * @brief Get dispatch table handle
     *
     * @param klass High-level class handle
     * @return Dispatch table handle
     */
    DispTableDesc class_disp_tab(const std::shared_ptr<Klass> &klass);

    /**
     * @brief Create initial value for a given type
     *
     * @param type Type
     * @return Initial value
     */
    Value init_value(const std::shared_ptr<ast::Type> &type);

    /**
     * @brief Emit code to file
     *
     * @param out_file Output file name
     */
    void emit(const std::string &out_file);
};

}; // namespace codegen