#pragma once

#include "Assembler.h"
#include "ast/AST.h"
#include "codegen/namespace/NameSpace.h"
#include "semant/Semant.h"
#include "utils/Utils.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <unordered_map>

namespace codegen
{
/**
 * @brief Maintain data section
 *
 */
class DataSection
{
  private:
    static constexpr std::string_view CONST_BOOL_PREFIX = "bool_const";
    static constexpr std::string_view CONST_INT_PREFIX = "int_const";
    static constexpr std::string_view CONST_STRING_PREFIX = "str_const";

    static constexpr int32_t TRUE = 1;
    static constexpr int32_t FALSE = 0;

    static constexpr int INT_CONST_SIZE_IN_WORDS = 4;         // tag + size + disp_table + value
    static constexpr int BOOL_CONST_SIZE_IN_WORDS = 4;        // tag + size + disp_table + value
    static constexpr int STRING_CONST_BASE_SIZE_IN_WORDS = 4; // tag + size + disp_table + value

    const KlassBuilder &_builder;

    // constants
    std::unordered_map<bool, Label> _bool_constants; // (bool constant, constant mark), constants are in _code
    std::unordered_map<std::string, Label>
        _string_constants;                             // (string constant, constant mark), constants are in _code
    std::unordered_map<int32_t, Label> _int_constants; // (int constant, constant mark),  constants are in _code

    CodeBuffer _code; // final code
    Assembler _asm;   // main assembler

    // runtime tables
    const Label _class_obj_tab;

    // gather code
    void get_all_prototypes();
    void get_all_dispatch_tab();
    void get_class_obj_tab();
    void get_class_name_tab();

  public:
    /**
     * @brief Construct a new DataSection object
     *
     * @param builder KlassBuilder
     */
    DataSection(const KlassBuilder &builder);

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
    inline static int32_t true_value()
    {
        return TRUE;
    }

    /**
     * @brief Get the false value
     *
     * @return False value
     */
    inline static int32_t false_value()
    {
        return FALSE;
    }

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