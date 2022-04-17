#include "CodeGenMips.h"
#include "codegen/emitter/CodeGen.inline.h"
#include "codegen/emitter/data/Data.inline.h"

using namespace codegen;

#define __ _asm.

CodeGenMips::CodeGenMips(const std::shared_ptr<semant::ClassNode> &root)
    : CodeGen(std::make_shared<KlassBuilderMips>(root)), _asm(_code), _data(_builder, _runtime), _a0(Register::$a0),
      _s0(Register::$s0)
{
    DEBUG_ONLY(_table.set_printer([](const std::string &name, const Symbol &s) {
        LOG("Add symbol \"" + name + "\" with type " + ((s._type == Symbol::FIELD) ? "FIELD" : "LOCAL") +
            " and offset " + std::to_string(s._offset));
    }));

    // here we start generate methods code
    __ text_section();

    // we export some structures and method to global
    __ global(Label(Names::init_method(MainClassName)));
    __ global(Label(Names::init_method(BaseClassesNames[BaseClasses::INT])));
    __ global(Label(Names::init_method(BaseClassesNames[BaseClasses::STRING])));
    __ global(Label(Names::init_method(BaseClassesNames[BaseClasses::BOOL])));
    __ global(Label(_builder->klass(MainClassName)->method_full_name(MainMethodName)));
}

void CodeGenMips::emit(const std::string &out_file_name)
{
    const std::string asm_file = out_file_name + static_cast<std::string>(EXT);

    emit_class_code(_builder->root()); // emit

    _data.emit(asm_file); // record data

    std::ofstream out_file(asm_file, std::ios::app); // open file

    Assembler::check_labels(); // verify that all labels were binded

    out_file << static_cast<std::string>(_code); // record code
}

void CodeGenMips::add_fields()
{
    const auto &this_klass = _builder->klass(_current_class->_type->_string);
    for (auto field = this_klass->fields_begin(); field != this_klass->fields_end(); field++)
    {
        const auto &name = (*field)->_object->_object;
        const auto offset = this_klass->field_offset(field - this_klass->fields_begin());
        // save fields to local symbol table for further CodeGenMips
        Symbol s(Symbol::FIELD, offset);
        _table.add_symbol(name, s);
    }
}

void CodeGenMips::emit_binary_expr_inner(const ast::BinaryExpression &expr)
{
    emit_expr(expr._lhs);
    // we hope to see the first argument in acc
    __ push(_a0);
    emit_expr(expr._rhs);

    auto logical_result = false;
    const Register t1(Register::$t1); // allocate new temp reg
    const Register t2(Register::$t2);
    const Register t5(Register::$t5);

    // rhs in acc, load lhs to t0
    __ pop(t1);
    // for arith operations create copy of the rhs for result
    __ move(t2, _a0);
    // operand are in t2 and t1, use t5 for result
    if (!std::holds_alternative<ast::EqExpression>(expr._base))
    {
        emit_load_int(t2, t2); // load real values
        emit_load_int(t1, t1);

        std::visit(ast::overloaded{[&](const ast::MinusExpression &minus) { __ sub(t5, t1, t2); },
                                   [&](const ast::PlusExpression &plus) { __ add(t5, t1, t2); },
                                   [&](const ast::DivExpression &div) { __ div(t5, t1, t2); },
                                   [&](const ast::MulExpression &mul) { __ mul(t5, t1, t2); },
                                   [&](const ast::LTExpression &lt) {
                                       // trait it as arith result
                                       __ slt(t5, t1, t2);
                                       __ la(_a0, _data.bool_const(false));
                                   },
                                   [&](const ast::LEExpression &le) {
                                       logical_result = true;

                                       const Label true_label(Names::true_branch());
                                       const Label end_label(Names::merge_block());

                                       __ ble(t1, t2, true_label);          // t1 < a0
                                       __ la(_a0, _data.bool_const(false)); // false -> a0 if false
                                       __ j(end_label);                     // jump to end

                                       {
                                           const AssemblerMarkSection mark(_asm, true_label); // true path
                                           __ la(_a0, _data.bool_const(true));
                                       }

                                       const AssemblerMarkSection mark(_asm, end_label); // jump here to continue
                                   },
                                   [&](const ast::EqExpression &le) {}},
                   expr._base);
    }
    else
    {
        logical_result = true;

        const Label equal_refs_label(Names::true_branch());
        const Label equal_primitive_vals_label(Names::true_branch());
        const Label equal_end_label(Names::merge_block());

        const Register a1(Register::$a1);

        __ beq(t2, t1, equal_refs_label); // are they the same reference?
        // no, they dont have the same reference
        __ la(_a0, _data.bool_const(true)); // if no, set a0 to true, a1 to false
        __ la(a1, _data.bool_const(false));
        __ jal(*_runtime.method(RuntimeMips::EQUALITY_TEST)); // in a0 expect a0 if the same and a1 if false
        emit_load_bool(_a0, _a0);                             // real value in a0
        __ beq(_a0, TrueValue, equal_primitive_vals_label);   // do they have same type and value?
        // no, they dont have the same type and value
        __ la(_a0, _data.bool_const(false)); // end of false branch
        __ j(equal_end_label);               // jump to end

        {
            // yes, they have the same reference
            const AssemblerMarkSection mark(_asm, equal_refs_label);
            __ la(_a0, _data.bool_const(true)); // end of true branch
            __ j(equal_end_label);              // jump to end
        }

        {
            // yes, they have the same type and value
            const AssemblerMarkSection mark(_asm, equal_primitive_vals_label);
            __ la(_a0, _data.bool_const(true));
        }

        const AssemblerMarkSection mark(_asm, equal_end_label); // jump here to continue
    }

    // object in a0, result in t5
    // create object and set field
    __ jal(*_runtime.method(RuntimeMips::OBJECT_COPY));
    // result in a0
    if (!logical_result)
    {
        emit_store_int(_a0, t5);
    }
}

void CodeGenMips::emit_unary_expr_inner(const ast::UnaryExpression &expr)
{
    emit_expr(expr._expr);

    auto logical_result = false;
    // result in acc
    const Register t5(Register::$t5);
    std::visit(ast::overloaded{[&](const ast::IsVoidExpression &isvoid) {
                                   logical_result = true;

                                   const Label its_void_label(Names::true_branch());
                                   const Label end_label_label(Names::merge_block());

                                   __ beq(_a0, __ zero(), its_void_label);
                                   // false branch
                                   __ la(_a0, _data.bool_const(false)); // end of false branch
                                   __ j(end_label_label);               // jump to end
                                   // true branch
                                   {
                                       const AssemblerMarkSection mark(_asm, its_void_label);
                                       __ la(_a0, _data.bool_const(true)); // end of true branch
                                   }

                                   const AssemblerMarkSection mark(_asm, end_label_label);
                               },
                               [&](const ast::NotExpression &) {
                                   // trait it as arith result
                                   __ move(t5, _a0);
                                   emit_load_bool(t5, t5);
                                   __ xori(t5, t5, TrueValue);
                                   __ la(_a0, _data.bool_const(false));
                               },
                               [&](const ast::NegExpression &neg) {
                                   __ move(t5, _a0);
                                   emit_load_int(t5, t5);
                                   __ sub(t5, __ zero(), t5);
                               }},
               expr._base);
    // object in a0, result in t5
    // create object and set field
    __ jal(*_runtime.method(RuntimeMips::OBJECT_COPY));
    // result in a0
    if (!logical_result)
    {
        emit_store_int(_a0, t5);
    }
}

void CodeGenMips::emit_method_prologue()
{
    __ addiu(__ sp(), __ sp(), -(WORD_SIZE * 3));
    __ sw(__ fp(), __ sp(), (WORD_SIZE * 3));
    __ sw(_s0, __ sp(), (WORD_SIZE * 2));
    __ sw(__ ra(), __ sp(), WORD_SIZE);
    __ addiu(__ fp(), __ sp(), WORD_SIZE * 3); // 4 for fp, 4 fo ra, 4 for s0
    __ move(_s0, _a0);                         // "this" in a0

    _asm.set_sp_offset(-(WORD_SIZE * 3));
}

void CodeGenMips::emit_method_epilogue(const int &params_num)
{
    __ lw(__ ra(), __ sp(), WORD_SIZE);
    __ lw(_s0, __ sp(), (2 * WORD_SIZE));
    __ lw(__ fp(), __ sp(), (3 * WORD_SIZE));
    __ addiu(__ sp(), __ sp(), (params_num + 3) * WORD_SIZE); // pop all arguments to preserve stack
    __ jr(__ ra());

    _asm.set_sp_offset(0);
}

void CodeGenMips::emit_class_init_method_inner()
{
    const AssemblerMarkSection mark(_asm, Label(Names::init_method(_current_class->_type->_string)));

    emit_method_prologue();

    if (!semant::Semant::is_empty_type(_current_class->_parent)) // Object moment
    {
        __ jal(Label(Names::init_method(
            _current_class->_parent->_string))); // receiver already is in acc, call parent constructor
    }

    for (const auto &feature : _current_class->_features)
    {
        if (std::holds_alternative<ast::AttrFeature>(feature->_base))
        {
            if (feature->_expr)
            {
                emit_expr(feature->_expr); // calculate expression

                const auto &this_field = _table.symbol(feature->_object->_object);
                GUARANTEE_DEBUG(this_field._type == Symbol::FIELD); // impossible

                __ sw(_a0, _s0, this_field._offset); // save result to field in object
            }
        }
    }

    __ move(_a0, _s0); // return "this"
    emit_method_epilogue(0);
}

void CodeGenMips::emit_class_method_inner(const std::shared_ptr<ast::Feature> &method)
{
    const auto &class_name = _current_class->_type->_string;
    const auto &method_name = method->_object->_object;

    // it is dummies for basic classes. There are external symbols
    if (semant::Semant::is_basic_type(_current_class->_type))
    {
        Label(_builder->klass(class_name)->method_full_name(method_name), Label::ALLOW_NO_BIND);
        return;
    }

    const AssemblerMarkSection mark(_asm, Label(_builder->klass(class_name)->method_full_name(method_name)));

    emit_method_prologue();

    // add formals to symbol table
    _table.push_scope();
    const auto &params = std::get<ast::MethodFeature>(method->_base)._formals;
    const auto params_num = params.size();
    for (auto i = 0; i < params_num; i++)
    {
        const auto offset = (params_num - i) * WORD_SIZE;

        Symbol s(Symbol::LOCAL, offset);
        _table.add_symbol(params[i]->_object->_object, s); // map formal parameters to arguments
    }

    emit_expr(method->_expr);
    _table.pop_scope();

    emit_method_epilogue(params_num);
}

void CodeGenMips::emit_bool_expr(const ast::BoolExpression &expr)
{
    __ la(_a0, _data.bool_const(expr._value));
}

void CodeGenMips::emit_int_expr(const ast::IntExpression &expr)
{
    __ la(_a0, _data.int_const(expr._value));
}

void CodeGenMips::emit_string_expr(const ast::StringExpression &expr)
{
    __ la(_a0, _data.string_const(expr._string));
}

void CodeGenMips::emit_object_expr_inner(const ast::ObjectExpression &expr)
{
    if (!semant::Scope::can_assign(expr._object))
    {
        __ move(_a0, _s0); // self object: just copy to acc
        return;
    }

    const auto &object = _table.symbol(expr._object);
    if (object._type == Symbol::FIELD)
    {
        __ lw(_a0, _s0, object._offset); // object field
    }
    else
    {
        __ lw(_a0, __ fp(), object._offset); // local object defined in let, case, formal parameter
    }
}

void CodeGenMips::emit_new_expr_inner(const ast::NewExpression &expr)
{
    // we know the type
    if (!semant::Semant::is_self_type(expr._type))
    {
        const auto &class_name = _builder->klass(expr._type->_string)->name();

        __ la(_a0, Label(Names::prototype(class_name)));
        __ jal(*_runtime.method(RuntimeMips::OBJECT_COPY)); // result in acc
        __ jal(Label(Names::init_method(class_name)));      // result in acc
    }
    else
    {
        // we dont know the type
        Register t5(Register::$t5);
        Register t6(Register::$t6);

        __ lw(t6, _s0, 0);                                       // load tag of "this"
        __ sll(t5, t6, OBJECT_HEADER_SIZE_IN_BYTES / WORD_SIZE); // calculate offset in object table
        __ la(t6, _runtime.class_obj_tab());                     // load object table
        __ addu(t5, t6, t5);                                     // find protobj position in table

        __ lw(_a0, t5, 0);
        __ jal(*_runtime.method(RuntimeMips::OBJECT_COPY)); // result in acc

        __ lw(t5, t5, WORD_SIZE); // next slot is init method
        __ jalr(t5);
    }
}

void CodeGenMips::emit_in_scope(const std::shared_ptr<ast::ObjectExpression> &object,
                                const std::shared_ptr<ast::Type> &object_type,
                                const std::shared_ptr<ast::Expression> &expr, const bool &assign_acc)
{
    _table.push_scope();

    Symbol s(Symbol::LOCAL, __ sp_offset());
    _table.add_symbol(object->_object, s);

    if (assign_acc)
    {
        __ push(_a0); // save acc to new slot
    }
    else
    {
        if (semant::Semant::is_trivial_type(object_type))
        {
            __ la(_a0, _data.init_value(object_type));
            __ push(_a0);
        }
        else
        {
            __ push(__ zero()); // defualt value for uninitialized objects
        }
    }

    emit_expr(expr);

    _table.pop_scope();
    __ pop(); // delete slot
}

void CodeGenMips::emit_cases_expr_inner(const ast::CaseExpression &expr)
{
    emit_expr(expr._expr);

    auto case_branch_name = Names::true_branch();
    const auto no_branch_name = Names::false_branch();
    const Label continue_label(Names::merge_block());

    // in brackets because we want to deallocate t1 before next expressions emitting
    {
        const Register t1(Register::$t1);

        __ bne(_a0, __ zero(), Label(case_branch_name)); // is expression void
        // yes, it is void
        __ la(_a0, _data.string_const(_current_class->_file_name)); // save file name to a0
        __ li(t1, expr._expr->_line_number);                        // save line number to t1
        __ jal(*_runtime.method(RuntimeMips::CASE_ABORT2));         // abort
    }

    // we want to generate code for the the most precise cases first, so sort cases by tag
    auto cases = expr._cases;
    std::sort(cases.begin(), cases.end(), [&](const auto &case_a, const auto &case_b) {
        return _builder->tag(case_b->_type->_string) < _builder->tag(case_a->_type->_string);
    });

    // no, it is not void
    // Last case is a special case: branch to abort
    for (auto i = 0; i < cases.size(); i++)
    {
        const AssemblerMarkSection mark(_asm, Label(case_branch_name));
        {
            const Register t1(Register::$t1);

            __ lw(t1, _a0, 0); // if we here, so object is in acc. Load its tag to t1

            case_branch_name = (i < cases.size() - 1 ? Names::true_branch() : no_branch_name);

            const auto &klass = _builder->klass(cases[i]->_type->_string);

            __ blt(
                t1, klass->tag(),
                Label(case_branch_name)); // if object tag lower than the lowest tag for this branch, jump to next case
            __ bgt(
                t1, klass->child_max_tag(),
                Label(
                    case_branch_name)); // if object tag higher that the highest tag for this branch, jump to next case
        }
        // this branch is ok. Object is in acc
        emit_in_scope(cases[i]->_object, cases[i]->_type, cases[i]->_expr);
        __ j(continue_label);
    }

    // does not find suitable branch

    {
        const AssemblerMarkSection mark(_asm, Label(no_branch_name));
        __ jal(*_runtime.method(RuntimeMips::CASE_ABORT)); // abort
    }

    const AssemblerMarkSection mark(_asm, continue_label);
}

void CodeGenMips::emit_let_expr_inner(const ast::LetExpression &expr)
{
    if (expr._expr)
    {
        emit_expr(expr._expr); // result in acc
    }

    emit_in_scope(expr._object, expr._type, expr._body_expr, expr._expr != nullptr);
}

void CodeGenMips::emit_branch_to_label_if_false(const Label &label)
{
    emit_load_bool(_a0, _a0);
    __ beq(_a0, FalseValue, label);
}

void CodeGenMips::emit_loop_expr_inner(const ast::WhileExpression &expr)
{
    const Label loop_header_label(Names::loop_header());
    const Label loop_tail_label(Names::loop_tail());

    {
        const AssemblerMarkSection mark(_asm, loop_header_label);

        emit_expr(expr._predicate); // result in acc
        emit_branch_to_label_if_false(loop_tail_label);
        // loop body
        emit_expr(expr._body_expr);
        __ j(loop_header_label); // go to loop start
    }

    const AssemblerMarkSection mark(_asm, loop_tail_label); // continue
}

void CodeGenMips::emit_if_expr_inner(const ast::IfExpression &expr)
{
    const Label false_branch_label(Names::false_branch());
    const Label continue_label(Names::merge_block());

    emit_expr(expr._predicate); // result in acc
    emit_branch_to_label_if_false(false_branch_label);
    // true branch
    emit_expr(expr._true_path_expr);
    __ j(continue_label); // continue execution

    {
        // false branch
        const AssemblerMarkSection mark(_asm, false_branch_label);

        emit_expr(expr._false_path_expr);
    }

    const AssemblerMarkSection mark(_asm, continue_label);
}

void CodeGenMips::emit_dispatch_expr_inner(const ast::DispatchExpression &expr)
{
    // put all args on stack. Callee have to get rid of them
    const auto args_num = expr._args.size();
    // allocate space
    __ addiu(__ sp(), __ sp(), -(args_num * WORD_SIZE));
    for (auto i = 0; i < args_num; i++)
    {
        emit_expr(expr._args[i]);
        __ sw(_a0, __ sp(), (args_num - i) * WORD_SIZE);
    }

    emit_expr(expr._expr); // receiver in acc

    const Label dispatch_to_void_label(Names::true_branch());
    const Register t1(Register::$t1);
    __ beq(_a0, __ zero(), dispatch_to_void_label);

    const auto &method_name = expr._object->_object;
    // not void
    std::visit(
        ast::overloaded{
            [&](const ast::VirtualDispatchExpression &disp) {
                __ lw(t1, _a0, DISPATCH_TABLE_OFFSET); // load dispatch table
                __ lw(t1, t1,
                      _builder->klass(semant::Semant::exact_type(expr._expr->_type, _current_class->_type)->_string)
                              ->method_index(method_name) *
                          WORD_SIZE); // load method label
                __ jalr(t1);          // jump to method
            },
            [&](const ast::StaticDispatchExpression &disp) {
                // we know exactly method name
                __ jal(Label(_builder->klass(disp._type->_string)->method_full_name(method_name)));
            }},
        expr._base);
    const Label continue_label(Names::merge_block());
    __ j(continue_label);

    // void
    {
        const AssemblerMarkSection mark(_asm, dispatch_to_void_label);
        __ li(t1, expr._expr->_line_number);
        __ la(_a0, _data.string_const(_current_class->_file_name));
        __ jal(*_runtime.method(RuntimeMips::DISPATCH_ABORT));
    }

    const AssemblerMarkSection mark(_asm, continue_label);
}

void CodeGenMips::emit_assign_expr_inner(const ast::AssignExpression &expr)
{
    emit_expr(expr._expr); // result in acc
    const auto &symbol = _table.symbol(expr._object->_object);

    if (symbol._type == Symbol::FIELD)
    {
        __ sw(_a0, _s0, symbol._offset);
        emit_gc_update(_s0, symbol._offset);
    }
    else
    {
        __ sw(_a0, __ fp(), symbol._offset);
    }
}

void CodeGenMips::emit_load_int(const Register &int_obj, const Register &int_val)
{
    __ lw(int_val, int_obj, OBJECT_HEADER_SIZE_IN_BYTES);
}

void CodeGenMips::emit_store_int(const Register &int_obj, const Register &int_val)
{
    __ sw(int_val, int_obj, OBJECT_HEADER_SIZE_IN_BYTES);
    emit_gc_update(int_obj, OBJECT_HEADER_SIZE_IN_BYTES);
}

void CodeGenMips::emit_load_bool(const Register &bool_obj, const Register &bool_val)
{
    emit_load_int(bool_obj, bool_val);
}

void CodeGenMips::emit_store_bool(const Register &bool_obj, const Register &bool_val)
{
    emit_store_int(bool_obj, bool_val);
}

void CodeGenMips::emit_gc_update(const Register &obj, const int &offset)
{
    Register a1(Register::$a1);
    __ addiu(a1, obj, offset);
    __ jal(*_runtime.method(RuntimeMips::GEN_GC_ASSIGN));
}