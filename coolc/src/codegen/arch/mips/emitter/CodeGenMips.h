#pragma once

#include "codegen/arch/mips/klass/KlassMips.h"
#include "codegen/arch/mips/symtab/SymbolTableMips.h"
#include "codegen/emitter/CodeGen.h"
#include "data/DataMips.h"

namespace codegen
{

/**
 * @brief CodeGenMips emits code for SPIM
 *
 */
class CodeGenMips : public CodeGen<void, Symbol>
{
  private:
    // usefull constants
    static constexpr int OBJECT_HEADER_SIZE_IN_BYTES = 3 * WORD_SIZE;
    static constexpr int DISPATCH_TABLE_OFFSET = 2 * WORD_SIZE;

    CodeBuffer _code;     // main code buffer for main assembler
    Assembler _asm;       // main assembler
    RuntimeMips _runtime; // Runtime methods and tables
    DataMips _data;       // Data structures management

    // convention values and reserved registers
    const Register _a0; // accumulator
    const Register _s0; // receiver or "this"

    void add_fields() override;

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

    void emit_class_method_inner(const std::shared_ptr<ast::Feature> &method) override;
    void emit_class_init_method_inner() override;

    void emit_binary_expr_inner(const ast::BinaryExpression &expr) override;
    void emit_unary_expr_inner(const ast::UnaryExpression &expr) override;
    void emit_bool_expr(const ast::BoolExpression &expr) override;
    void emit_int_expr(const ast::IntExpression &expr) override;
    void emit_string_expr(const ast::StringExpression &expr) override;
    void emit_object_expr_inner(const ast::ObjectExpression &expr) override;
    void emit_new_expr_inner(const ast::NewExpression &expr) override;

    // allocate stack slot for object, assign acc value to it, evaluate expression, delete slot after that
    void emit_in_scope(const std::shared_ptr<ast::ObjectExpression> &object,
                       const std::shared_ptr<ast::Type> &object_type, const std::shared_ptr<ast::Expression> &expr,
                       const bool &assign_acc = true);

    void emit_cases_expr_inner(const ast::CaseExpression &expr) override;
    void emit_let_expr_inner(const ast::LetExpression &expr) override;

    // if bool result in acc is false - branch to label
    void emit_branch_to_label_if_false(const Label &label);

    void emit_loop_expr_inner(const ast::WhileExpression &expr) override;
    void emit_if_expr_inner(const ast::IfExpression &expr) override;
    void emit_dispatch_expr_inner(const ast::DispatchExpression &expr) override;
    void emit_assign_expr_inner(const ast::AssignExpression &expr) override;

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
    explicit CodeGenMips(const std::shared_ptr<semant::ClassNode> &root);

    void emit(const std::string &out_file) override;
};

}; // namespace codegen