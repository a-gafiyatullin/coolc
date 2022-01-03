#pragma once

#include <unordered_map>
#include <cmath>
#include <algorithm>
#include <cassert>
#include "Assembler.h"
#include "ast/AST.h"
#include "semant/Semant.h"

#ifdef CODEGEN_FULL_VERBOSE
#include "utils/logger/Logger.h"
#endif // CODEGEN_FULL_VERBOSE

namespace codegen
{
    class DataSection;

    // code associated with specific class
    class ClassCode
    {
    private:
        static constexpr int _OBJECT_BASE_SIZE_IN_WORDS = 3; // tag + size + disp_table

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

#ifdef CODEGEN_FULL_VERBOSE
        Logger _logger;

        inline void set_parent_logger(Logger *logger) { _logger.set_parent_logger(logger); }
#endif // CODEGEN_FULL_VERBOSE

    public:
        ClassCode(const std::shared_ptr<ast::Type> &class_, const Label &label, DataSection &manager)
            : _class(class_), _class_name_const(label), _data(manager) {}

        // add parent fields to prototype
        void inherit_fields(const ClassCode &code);
        inline void set_prototype_field(const std::shared_ptr<ast::Type> &field_type) { _fields_types.push_back(field_type); }

        int get_method_index(const std::string &method_name) const;
        std::string get_method_full_name(const std::string &method_name) const;

        friend class DataSection;
    };

    class DataSection
    {
    private:
        static constexpr std::string_view _DISP_TAB_SUFFIX = "_dispTab";
        static constexpr std::string_view _PROTOTYPE_SUFFIX = "_protObj";
        static constexpr std::string_view _INIT_SUFFIX = "_init";

        static constexpr std::string_view _STRING_TYPE = "String";
        static constexpr std::string_view _BOOL_TYPE = "Bool";
        static constexpr std::string_view _INT_TYPE = "Int";

        static constexpr std::string_view _CONST_BOOL_PREFIX = "bool_const";
        static constexpr std::string_view _CONST_INT_PREFIX = "int_const";
        static constexpr std::string_view _CONST_STRING_PREFIX = "str_const";

        static constexpr int32_t _TRUE = 1;
        static constexpr int32_t _FALSE = 0;

        static constexpr int _INT_CONST_SIZE_IN_WORDS = 4;         // tag + size + disp_table + value
        static constexpr int _BOOL_CONST_SIZE_IN_WORDS = 4;        // tag + size + disp_table + value
        static constexpr int _STRING_CONST_BASE_SIZE_IN_WORDS = 4; // tag + size + disp_table + value

        // constants
        std::unordered_map<bool, Label> _bool_constants;          // (bool constant, constant mark), constants are in _code
        std::unordered_map<std::string, Label> _string_constants; // (string constant, constant mark), constants are in _code
        std::unordered_map<int32_t, Label> _int_constants;        // (int constant, constant mark),  constants are in _code

        std::unordered_map<std::string, std::pair<int, int>> _tags; // (class name, (tag, max child tag))
        std::unordered_map<int, ClassCode> _class_codes;            // code associated with classes

        CodeBuffer _code; // final code
        Assembler _asm;   // main assembler

        // runtime tables
        const Label _class_obj_tab;

        // create new tag or return known
        int create_tag(const std::string &type);

        // basic type tags
        inline int get_bool_tag() { return get_tag(static_cast<std::string>(_BOOL_TYPE)); }
        inline int get_string_tag() { return get_tag(static_cast<std::string>(_STRING_TYPE)); }
        inline int get_int_tag() { return get_tag(static_cast<std::string>(_INT_TYPE)); }

        // gather code
        void get_all_prototypes();
        void get_all_dispatch_tab();
        void get_class_obj_tab();
        void get_class_name_tab();

        int build_class_info(const std::shared_ptr<semant::ClassNode> &root); // emit necessary tables, find max child tag

        ClassCode &create_class_code(const std::shared_ptr<ast::Type> &class_);

        CODEGEN_FULL_VERBOSE_ONLY(Logger _logger);

    public:
        DataSection(const std::shared_ptr<semant::ClassNode> &root);

        // accessors
        ClassCode &get_class_code(const std::shared_ptr<ast::Type> &class_);
        const Label &class_obj_tab() const { return _class_obj_tab; }

        // declare consts
        const Label &declare_string_const(const std::string &str);
        const Label &declare_bool_const(const bool &value);
        const Label &declare_int_const(const int32_t &value);

        // false and true value
        inline static int32_t get_true_value() { return _TRUE; }
        inline static int32_t get_false_value() { return _FALSE; }

        // return known tag
        int get_tag(const std::string &type) const;
        int get_max_child_tag(const std::string &type) const;

        // construct names
        inline static std::string get_disp_table_name(const std::string &class_name) { return class_name + static_cast<std::string>(_DISP_TAB_SUFFIX); }
        inline static std::string get_prototype_name(const std::string &class_name) { return class_name + static_cast<std::string>(_PROTOTYPE_SUFFIX); }
        inline static std::string get_init_method_name(const std::string &class_name) { return class_name + static_cast<std::string>(_INIT_SUFFIX); }
        static std::string get_full_method_name(const std::string &class_name, const std::string &method);

        static const Label &emit_init_value(Assembler &_asm, DataSection &_data, const std::shared_ptr<ast::Type> &type);

        CodeBuffer emit();

        inline Assembler &get_asm() { return _asm; }

#ifdef CODEGEN_FULL_VERBOSE
        inline void set_parent_logger(Logger *logger) { _logger.set_parent_logger(logger); }
#endif // CODEGEN_FULL_VERBOSE
    };
};