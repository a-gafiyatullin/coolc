#pragma once

#include "SymbolTable.h"
#include "DataSection.h"

#ifdef CODEGEN_FULL_VERBOSE
#include "utils/logger/Logger.h"
#endif // CODEGEN_FULL_VERBOSE

namespace codegen
{
    class CodeGen
    {
    private:
        // usefull constants
        static constexpr int _WORD_SIZE = 4;
        static constexpr int _OBJECT_HEADER_SIZE_IN_BYTES = 3 * _WORD_SIZE;
        static constexpr int _DISPATCH_TABLE_OFFSET = 2 * _WORD_SIZE;

        // result asm code. Other CodeBuffers will be merged here
        CodeBuffer _code;  // main code buffer for main assembler
        DataSection _data; // manage class prototypes, tables, constants and other such stuff
        Assembler _asm;    // main assembler

        // runtime calls
        const Label _equality_test_label;
        const Label _object_copy_label;
        const Label _case_abort_label;
        const Label _case_abort2_label;
        const Label _dispatch_abort_label;
        const Label _genGC_assign;

        // convention values and reserved registers
        const Register _a0; // accumulator
        const Register _s0; // receiver or "this"

        // current generating class
        std::shared_ptr<ast::Class> _current_class;

        // labels creation
        static constexpr std::string_view _LABEL_NAME = "label";
        int _label_num;                                                                                                      // last label number
        inline std::string new_label_name() { return static_cast<std::string>(_LABEL_NAME) + std::to_string(_label_num++); } // create new label name "labelN"

        // program ast
        std::shared_ptr<semant::ClassNode> _root;

        // emit code for class, add fields to symbol table
        void emit_class_code(const std::shared_ptr<semant::ClassNode> &node, SymbolTable &table);
        void add_fields_to_table(const std::shared_ptr<ast::Class> &class_, SymbolTable &table);

        // methods of class
        /* Call convention:
         * 
         * caller saves arguments on stack in reverse order - the first argument on the top of the stack
         * caller calculates receiver and store it in acc
         * 
         * callee saves 1) fp, 2) s0 3) ra 4) set fp after the first parameter
         * callee does its stuff
         * callee restores 1) ra 2) s0 3) fp, 4) pops all parameters
         * 
         * result will be in acc
         */
        void emit_method_prologue();
        void emit_method_epilogue(const int &params_num);
        void emit_class_method(const std::shared_ptr<ast::Feature> &method, SymbolTable &table);
        void emit_class_init_method(const std::shared_ptr<ast::Class> &class_, SymbolTable &table);

        // emit expressions
        void emit_expr(const std::shared_ptr<ast::Expression> &expr, SymbolTable &table);
        void emit_binary_expr(const ast::BinaryExpression &expr, SymbolTable &table);
        void emit_unary_expr(const ast::UnaryExpression &expr, SymbolTable &table);
        void emit_bool_expr(const ast::BoolExpression &expr);
        void emit_int_expr(const ast::IntExpression &expr);
        void emit_string_expr(const ast::StringExpression &expr);
        void emit_object_expr(const ast::ObjectExpression &expr, SymbolTable &table);
        void emit_new_expr(const ast::NewExpression &expr);
        // allocate stack slot for object, assign acc value to it, evaluate expression, delete slot after that
        void emit_in_scope(const std::shared_ptr<ast::ObjectExpression> &object, const std::shared_ptr<ast::Type> &object_type,
                           const std::shared_ptr<ast::Expression> &expr, SymbolTable &table, const bool &assign_acc = true);
        void emit_cases_expr(const ast::CaseExpression &expr, SymbolTable &table);
        void emit_let_expr(const ast::LetExpression &expr, SymbolTable &table);
        void emit_list_expr(const ast::ListExpression &expr, SymbolTable &table);
        // if bool result in acc is false - branch to label
        void emit_branch_to_label_if_false(const Label &label);
        void emit_loop_expr(const ast::WhileExpression &expr, SymbolTable &table);
        void emit_if_expr(const ast::IfExpression &expr, SymbolTable &table);
        void emit_dispatch_expr(const ast::DispatchExpression &expr, SymbolTable &table);
        void emit_assign_expr(const ast::AssignExpression &expr, SymbolTable &table);

        // load/store basic values
        // int
        void emit_load_int(const Register &int_obj, const Register &int_val);
        void emit_store_int(const Register &int_obj, const Register &int_val);
        // bool
        void emit_load_bool(const Register &bool_obj, const Register &bool_val);
        void emit_store_bool(const Register &bool_obj, const Register &bool_val);

        void emit_gc_update(const Register &obj, const int &offset);

        CODEGEN_FULL_VERBOSE_ONLY(Logger _logger);

    public:
        // get root of program class hierarhy
        CodeGen(const std::shared_ptr<semant::ClassNode> &root);

        // main emitter method
        CodeBuffer emit();
    };
};