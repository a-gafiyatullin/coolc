#pragma once

#include "Assembler.h"
#include "ast/AST.h"
#include "semant/Semant.h"
#include "utils/Utils.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <memory>
#include <unordered_map>

#ifdef DEBUG
#include "utils/logger/Logger.h"
#endif // DEBUG

namespace codegen
{
class DataSection;

/**
 * @brief Code associated with specific class
 *
 */
class ClassCode
{
  private:
    static constexpr int OBJECT_BASE_SIZE_IN_WORDS = 3; // tag + size + disp_table

    const std::shared_ptr<ast::Type> _class; // name

    const Label _class_name_const;
    std::vector<std::string> _disp_table; // store strings during info collecting
    std::vector<std::shared_ptr<ast::Type>> _fields_types;

    DataSection &_data; // parent DataSection

    void construct_disp_table() const;
    void construct_prototype() const;

    // set new entry in the dispatch table or redefine previous
    void set_disp_table_entry(const std::string &method_name);
    void inherit_disp_table(const ClassCode &code);

  public:
    /**
     * @brief Construct a new ClassCode
     *
     * @param klass Class associated with this ClassCode
     * @param label Name of this ClassCode
     * @param manager DataSection that maintains this ClassCode
     */
    ClassCode(const std::shared_ptr<ast::Type> &klass, const Label &label, DataSection &manager)
        : _class(klass), _class_name_const(label), _data(manager)
    {
    }

    /**
     * @brief Add parent fields to prototype
     *
     * @param code Parent ClassCode
     */
    void inherit_fields(const ClassCode &code);

    /**
     * @brief Record next field type to prototype
     *
     * @param field_type Field type
     */
    inline void set_prototype_field(const std::shared_ptr<ast::Type> &field_type)
    {
        _fields_types.push_back(field_type);
    }

    /**
     * @brief Find index in dispatch table for given method
     *
     * @param method_name Name of the method
     * @return Index
     */
    int get_method_index(const std::string &method_name) const;

    /**
     * @brief Construct full name of the method for this ClassCode
     *
     * @param method_name Name of the method
     * @return Full name
     */
    std::string get_method_full_name(const std::string &method_name) const;

    friend class DataSection;
};

/**
 * @brief Maintain data section
 *
 */
class DataSection
{
  private:
    static constexpr std::string_view DISP_TAB_SUFFIX = "_dispTab";
    static constexpr std::string_view PROTOTYPE_SUFFIX = "_protObj";
    static constexpr std::string_view INIT_SUFFIX = "_init";

    static constexpr std::string_view STRING_TYPE = "String";
    static constexpr std::string_view BOOL_TYPE = "Bool";
    static constexpr std::string_view INT_TYPE = "Int";

    static constexpr std::string_view CONST_BOOL_PREFIX = "bool_const";
    static constexpr std::string_view CONST_INT_PREFIX = "int_const";
    static constexpr std::string_view CONST_STRING_PREFIX = "str_const";

    static constexpr int32_t TRUE = 1;
    static constexpr int32_t FALSE = 0;

    static constexpr int INT_CONST_SIZE_IN_WORDS = 4;         // tag + size + disp_table + value
    static constexpr int BOOL_CONST_SIZE_IN_WORDS = 4;        // tag + size + disp_table + value
    static constexpr int STRING_CONST_BASE_SIZE_IN_WORDS = 4; // tag + size + disp_table + value

    // constants
    std::unordered_map<bool, Label> _bool_constants; // (bool constant, constant mark), constants are in _code
    std::unordered_map<std::string, Label>
        _string_constants;                             // (string constant, constant mark), constants are in _code
    std::unordered_map<int32_t, Label> _int_constants; // (int constant, constant mark),  constants are in _code

    std::unordered_map<std::string, std::pair<int, int>> _tags; // (class name, (tag, max child tag))
    std::unordered_map<int, ClassCode> _class_codes;            // code associated with classes

    CodeBuffer _code; // final code
    Assembler _asm;   // main assembler

    // runtime tables
    const Label _class_obj_tab;

    // create new tag or return known
    int create_tag(const std::string &type);

    // basic type tags
    inline int get_bool_tag()
    {
        return get_tag(static_cast<std::string>(BOOL_TYPE));
    }
    inline int get_string_tag()
    {
        return get_tag(static_cast<std::string>(STRING_TYPE));
    }
    inline int get_int_tag()
    {
        return get_tag(static_cast<std::string>(INT_TYPE));
    }

    // gather code
    void get_all_prototypes();
    void get_all_dispatch_tab();
    void get_class_obj_tab();
    void get_class_name_tab();

    int build_class_info(const std::shared_ptr<semant::ClassNode> &root); // emit necessary tables, find max child tag

    ClassCode &create_class_code(const std::shared_ptr<ast::Type> &klass);

  public:
    /**
     * @brief Construct a new DataSection object
     *
     * @param root Root of the program
     */
    DataSection(const std::shared_ptr<semant::ClassNode> &root);

    /**
     * @brief Find ClassCode for a given class
     *
     * @param klass Class
     * @return ClassCode associated with a given class
     */
    ClassCode &get_class_code(const std::shared_ptr<ast::Type> &klass);

    /**
     * @brief Get Class Objects Table pointer
     *
     * @return Label of Class Objects Table
     */
    const Label &class_obj_tab() const
    {
        return _class_obj_tab;
    }

    /**
     * @brief Declare string constant
     *
     * @param str String
     * @return Label associated with a given string
     */
    const Label &declare_string_const(const std::string &str);

    /**
     * @brief Declare boolean constant
     *
     * @param value boolean
     * @return Label associated with a given bolean
     */
    const Label &declare_bool_const(const bool &value);

    /**
     * @brief Declare integer constant
     *
     * @param value integer
     * @return Label associated with a given integer
     */
    const Label &declare_int_const(const int32_t &value);

    /**
     * @brief Get the true value
     *
     * @return True value
     */
    inline static int32_t get_true_value()
    {
        return TRUE;
    }

    /**
     * @brief Get the false value
     *
     * @return False value
     */
    inline static int32_t get_false_value()
    {
        return FALSE;
    }

    /**
     * @brief Get the tag for a given class
     *
     * @param type Class
     * @return Tag
     */
    int get_tag(const std::string &type) const;

    /**
     * @brief Calculate maximal tag from all childs of a given class
     *
     * @param type Class
     * @return Tag
     */
    int get_max_child_tag(const std::string &type) const;

    /**
     * @brief Get the dispatch table name for a given class
     *
     * @param class_name Name of a class
     * @return Dispatch table name
     */
    inline static std::string get_disp_table_name(const std::string &class_name)
    {
        return class_name + static_cast<std::string>(DISP_TAB_SUFFIX);
    }

    /**
     * @brief Get the prototype name for a given class
     *
     * @param class_name Name of a class
     * @return Prototype name
     */
    inline static std::string get_prototype_name(const std::string &class_name)
    {
        return class_name + static_cast<std::string>(PROTOTYPE_SUFFIX);
    }

    /**
     * @brief Get the init method name for a given class
     *
     * @param class_name Name of a class
     * @return Init method name
     */
    inline static std::string get_init_method_name(const std::string &class_name)
    {
        return class_name + static_cast<std::string>(INIT_SUFFIX);
    }

    /**
     * @brief Construct full name of the method using class name
     *
     * @param class_name Name of a class
     * @param method Name of a method
     * @return Full method name
     */
    static std::string get_full_method_name(const std::string &class_name, const std::string &method);

    /**
     * @brief Create initial value for a given type
     *
     * @param type Tpe
     * @return Initial value label
     */
    const Label &emit_init_value(const std::shared_ptr<ast::Type> &type);

    /**
     * @brief Finish code emitting
     *
     * @return Final buffer
     */
    CodeBuffer emit();

    /**
     * @brief Get the Assembler
     *
     * @return Assembler
     */
    inline Assembler &get_asm()
    {
        return _asm;
    }
};
}; // namespace codegen