#pragma once

#include "DataSection.h"
#include "SymbolTableMips.h"
#include "codegen/emitter/CodeGen.h"
#include "codegen/namespace/NameSpace.h"
#include <fstream>

namespace codegen
{

/**
 * @brief CodeGenMips emits code for SPIM
 *
 */
class CodeGenMips : public CodeGen
{
  private:
    // usefull constants
    static constexpr int OBJECT_HEADER_SIZE_IN_BYTES = 3 * WORD_SIZE;
    static constexpr int DISPATCH_TABLE_OFFSET = 2 * WORD_SIZE;

    const KlassBuilder _builder;

    // result asm code. Other CodeBuffers will be merged here
    CodeBuffer _code;           // main code buffer for main assembler
    DataSection _data;          // manage class prototypes, tables, constants and other such stuff
    Assembler _asm;             // main assembler
    SymbolTable<Symbol> _table; // Symbol in scopes

    // runtime calls
    const Label _equality_test_label;
    const Label _object_copy_label;
    const Label _case_abort_label;
    const Label _case_abort2_label;
    const Label _dispatch_abort_label;
    const Label _gen_gc_assign_label;

    // convention values and reserved registers
    const Register _a0; // accumulator
    const Register _s0; // receiver or "this"

    // emit code for class, add fields to symbol table
    virtual void emit_class_code_inner(const std::shared_ptr<semant::ClassNode> &node);

    void add_fields_to_table(const std::shared_ptr<ast::Class> &klass);

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

    virtual void emit_class_method_inner(const std::shared_ptr<ast::Feature> &method);
    virtual void emit_class_init_method_inner(const std::shared_ptr<ast::Class> &klass);

    virtual void emit_binary_expr_inner(const ast::BinaryExpression &expr);
    virtual void emit_unary_expr_inner(const ast::UnaryExpression &expr);
    virtual void emit_bool_expr(const ast::BoolExpression &expr);
    virtual void emit_int_expr(const ast::IntExpression &expr);
    virtual void emit_string_expr(const ast::StringExpression &expr);
    virtual void emit_object_expr_inner(const ast::ObjectExpression &expr);
    virtual void emit_new_expr_inner(const ast::NewExpression &expr);

    // allocate stack slot for object, assign acc value to it, evaluate expression, delete slot after that
    void emit_in_scope(const std::shared_ptr<ast::ObjectExpression> &object,
                       const std::shared_ptr<ast::Type> &object_type, const std::shared_ptr<ast::Expression> &expr,
                       const bool &assign_acc = true);

    virtual void emit_cases_expr_inner(const ast::CaseExpression &expr);
    virtual void emit_let_expr_inner(const ast::LetExpression &expr);
    virtual void emit_list_expr_inner(const ast::ListExpression &expr);

    // if bool result in acc is false - branch to label
    void emit_branch_to_label_if_false(const Label &label);

    virtual void emit_loop_expr_inner(const ast::WhileExpression &expr);
    virtual void emit_if_expr_inner(const ast::IfExpression &expr);
    virtual void emit_dispatch_expr_inner(const ast::DispatchExpression &expr);
    virtual void emit_assign_expr_inner(const ast::AssignExpression &expr);

    // load/store basic values
    // int
    void emit_load_int(const Register &int_obj, const Register &int_val);
    void emit_store_int(const Register &int_obj, const Register &int_val);
    // bool
    void emit_load_bool(const Register &bool_obj, const Register &bool_val);
    void emit_store_bool(const Register &bool_obj, const Register &bool_val);

    void emit_gc_update(const Register &obj, const int &offset);

  public:
    /**
     * @brief Construct a new CodeGen object
     *
     * @param root Root of program class hierarhy
     */
    CodeGenMips(const std::shared_ptr<semant::ClassNode> &root);

    virtual void emit(const std::string &out_file);
};

}; // namespace codegen