#include "CodeGenMyIR.hpp"
#include "codegen/emitter/CodeGen.inline.h"
#include "codegen/emitter/data/Data.inline.h"

using namespace codegen;

#define __ _ir_builder.

CodeGenMyIR::CodeGenMyIR(const std::shared_ptr<semant::ClassNode> &root)
    : CodeGen(std::make_shared<KlassBuilderMyIR>(root)), _runtime(_module), _data(_builder, _module, _runtime),
      _ir_builder(_module), _true_val(myir::Constant::constant(myir::INT8, TrueValue)),
      _false_val(myir::Constant::constant(myir::INT8, FalseValue)), _true_obj(_data.bool_const(true)),
      _false_obj(_data.bool_const(false)), _null_val(myir::Constant::constant(myir::POINTER, 0))
{
    DEBUG_ONLY(_table.set_printer([](const std::string &name, const Symbol &s) {
        LOG("Added symbol \"" + name + "\": " + static_cast<std::string>(s))
    }));
}

void CodeGenMyIR::add_fields()
{
    auto &this_klass = _builder->klass(_current_class->_type->_string);

    for (auto field = this_klass->fields_begin(); field != this_klass->fields_end(); field++)
    {
        _table.add_symbol((*field)->_object->_object,
                          Symbol(this_klass->field_offset(field - this_klass->fields_begin()),
                                 semant::Semant::exact_type((*field)->_type, _current_class->_type)));
    }
}

void CodeGenMyIR::emit_class_method_inner(const std::shared_ptr<ast::Feature> &method)
{
    // it is dummies for basic classes. There are external symbols
    if (semant::Semant::is_basic_type(_current_class->_type))
    {
        return;
    }

    auto func = _module.get_function(
        _builder->klass(_current_class->_type->_string)->method_full_name(method->_object->_object));

    assert(func);

    __ set_current_function(func);

    // Create a new basic block to start insertion into it
    auto cfg = __ new_block(Names::name(Names::ENTRY_BLOCK));
    __ set_current_block(cfg);

    func->set_cfg(cfg);

    // add formals to symbol table
    _table.push_scope();

    auto &formals = std::get<ast::MethodFeature>(method->_base)._formals;

    for (auto i = 0; i < func->params_size(); i++)
    {
        auto arg = func->param(i);

        _table.add_symbol(arg->name(), Symbol(arg, i != 0 ? formals[i - 1]->_type : _current_class->_type));

        DEBUG_ONLY(verify_oop(arg));
    }

    __ ret(emit_expr(method->_expr));

    _table.pop_scope();
}

#ifdef DEBUG
void CodeGenMyIR::verify_oop(const myir::oper &object)
{
    // static auto VERIFY_OOP = _runtime.symbol_by_id(RuntimeMyIR::VERIFY_OOP)->_func;
    // if (VerifyOops)
    // {
    //     __ call(VERIFY_OOP, {object});
    // }
}
#endif // DEBUG

void CodeGenMyIR::emit_class_init_method_inner()
{
    auto &klass = _builder->klass(_current_class->_type->_string);

    // Note, that init method don't init header
    auto func = _module.get_function(klass->init_method());

    assert(func);

    __ set_current_function(func);

    // Create a new basic block to start insertion into
    auto cfg = __ new_block(Names::name(Names::ENTRY_BLOCK));
    __ set_current_block(cfg);

    func->set_cfg(cfg);

    // self is visible
    _table.push_scope();

    _table.add_symbol(SelfObject, Symbol(func->param(0), _current_class->_type));

    // set default value before init for fields of this class
    for (auto &feature : _current_class->_features)
    {
        if (std::holds_alternative<ast::AttrFeature>(feature->_base))
        {
            myir::oper initial_val = nullptr;

            auto &this_field = _table.symbol(feature->_object->_object);
            assert(this_field._type == Symbol::FIELD); // impossible

            if (semant::Semant::is_trivial_type(this_field._value_type))
            {
                initial_val = _data.init_value(this_field._value_type);
            }
            else if (!semant::Semant::is_native_type(this_field._value_type))
            {
                initial_val = _null_val;
            }
            else
            {
                assert(semant::Semant::is_basic_type(_current_class->_type));
                if (semant::Semant::is_native_int(this_field._value_type) ||
                    semant::Semant::is_native_bool(this_field._value_type))
                {
                    initial_val = myir::Constant::constant(myir::UINT64, 0);
                }
                else if (semant::Semant::is_native_string(this_field._value_type))
                {
                    initial_val = myir::Constant::constant(myir::INT8, 0);
                }
                else
                {
                    SHOULD_NOT_REACH_HERE();
                }
            }

            // it's ok to use param(0) here, because safepoint is impossible
            __ st(func->param(0), field_offset(this_field._offset), initial_val);
        }
    }

    // call parent constructor
    if (!semant::Semant::is_empty_type(_current_class->_parent)) // Object moment
    {
        auto parent = _builder->klass(_current_class->_parent->_string);

        __ call(_module.get_function(parent->init_method()), {func->param(0)});
    }

    // Now initialize
    for (auto &feature : _current_class->_features)
    {
        if (std::holds_alternative<ast::AttrFeature>(feature->_base))
        {
            if (feature->_expr)
            {
                auto &this_field = _table.symbol(feature->_object->_object);
                assert(this_field._type == Symbol::FIELD); // impossible

                auto value = emit_expr(feature->_expr);
                auto self = func->param(0);

                __ st(self, field_offset(this_field._offset), value);
            }
        }
    }

    __ ret(nullptr);

    _table.pop_scope();
}

void CodeGenMyIR::make_control_flow(const myir::oper &pred, myir::block &true_block, myir::block &false_block,
                                    myir::block &merge_block)
{
    true_block = __ new_block(Names::name(Names::TRUE_BRANCH));
    false_block = __ new_block(Names::name(Names::FALSE_BRANCH));
    merge_block = __ new_block(Names::name(Names::MERGE_BLOCK));

    __ cond_br(pred, true_block, false_block);

    __ set_current_block(true_block);
}

myir::oper CodeGenMyIR::emit_ternary_operator(const myir::oper &pred, const myir::oper &true_val,
                                              const myir::oper &false_val)
{
    auto result = __ move(true_val);

    myir::block true_block = nullptr, false_block = nullptr, merge_block = nullptr;
    make_control_flow(pred, true_block, false_block, merge_block);

    // true block
    __ move(true_val, result);
    __ br(merge_block);

    // false block
    __ set_current_block(false_block);
    __ move(false_val, result);
    __ br(merge_block);

    // merge block
    __ set_current_block(merge_block);

    return result;
}

myir::oper CodeGenMyIR::emit_binary_expr_inner(const ast::BinaryExpression &expr,
                                               const std::shared_ptr<ast::Type> &expr_type)
{
    auto lhs = emit_expr(expr._lhs);
    auto rhs = emit_expr(expr._rhs);

    DEBUG_ONLY(verify_oop(lhs));
    DEBUG_ONLY(verify_oop(rhs));

    auto logical_result = false;
    myir::oper op_result = nullptr;

    if (!std::holds_alternative<ast::EqExpression>(expr._base))
    {
        auto lv = emit_load_int(lhs); // load real values
        auto rv = emit_load_int(rhs);

        op_result = std::visit(
            ast::overloaded{[&](const ast::MinusExpression &minus) { return __ sub(lv, rv); },
                            [&](const ast::PlusExpression &plus) { return __ add(lv, rv); },
                            [&](const ast::DivExpression &div) { return __ div(lv, rv); },
                            [&](const ast::MulExpression &mul) { return __ mul(lv, rv); },
                            [&](const ast::LTExpression &lt) {
                                logical_result = true;
                                return emit_ternary_operator(__ lt(lv, rv), _true_obj, _false_obj);
                            },
                            [&](const ast::LEExpression &le) {
                                logical_result = true;
                                return emit_ternary_operator(__ le(lv, rv), _true_obj, _false_obj);
                            },
                            [&](const ast::EqExpression &le) { return std::shared_ptr<myir::Operand>(nullptr); }},
            expr._base);
    }
    else
    {
        logical_result = true;

        auto result = __ move(_true_obj);

        auto is_same_ref = __ eq(lhs, rhs);

        myir::block true_block = nullptr, false_block = nullptr, merge_block = nullptr;
        make_control_flow(is_same_ref, true_block, false_block, merge_block);

        // true branch
        __ set_current_block(true_block);
        __ move(_true_obj, result);
        __ br(merge_block);

        // false branch - runtime call to equals
        __ set_current_block(false_block);

        auto equals_func = _runtime.symbol_by_id(RuntimeMyIR::RuntimeMyIRSymbols::EQUALS)->_func;
        auto eq_call_res = __ call(equals_func, {lhs, rhs});

        auto false_branch_res = emit_ternary_operator(__ eq(eq_call_res, _true_val), _true_obj, _false_obj);

        __ move(false_branch_res, result);
        __ br(merge_block);

        // merge results
        __ set_current_block(merge_block);
        op_result = result;
    }

    return logical_result ? op_result : emit_allocate_int(op_result);
}

myir::oper CodeGenMyIR::emit_unary_expr_inner(const ast::UnaryExpression &expr,
                                              const std::shared_ptr<ast::Type> &expr_type)
{
    auto operand = emit_expr(expr._expr);

    DEBUG_ONLY(verify_oop(operand));

    return std::visit(
        ast::overloaded{
            [&](const ast::IsVoidExpression &isvoid) {
                return emit_ternary_operator(__ eq(operand, _null_val), _true_obj, _false_obj);
            },
            [&](const ast::NotExpression &) {
                auto not_res_val = __ xor2(emit_load_bool(operand), _true_val);

                return emit_ternary_operator(__ eq(not_res_val, _true_val), _true_obj, _false_obj);
            },
            [&](const ast::NegExpression &neg) { return emit_allocate_int(__ neg(emit_load_int(operand))); }},
        expr._base);
}

myir::oper CodeGenMyIR::emit_bool_expr(const ast::BoolExpression &expr, const std::shared_ptr<ast::Type> &expr_type)
{
    return _data.bool_const(expr._value);
}

myir::oper CodeGenMyIR::emit_int_expr(const ast::IntExpression &expr, const std::shared_ptr<ast::Type> &expr_type)
{
    return _data.int_const(expr._value);
}

myir::oper CodeGenMyIR::emit_string_expr(const ast::StringExpression &expr, const std::shared_ptr<ast::Type> &expr_type)
{
    return _data.string_const(expr._string);
}

myir::oper CodeGenMyIR::emit_object_expr_inner(const ast::ObjectExpression &expr,
                                               const std::shared_ptr<ast::Type> &expr_type)
{
    auto &object = _table.symbol(expr._object);

    if (object._type == Symbol::FIELD)
    {
        return __ ld<myir::POINTER>(emit_load_self(), field_offset(object._offset));
    }

    return object._variable;
}

myir::oper CodeGenMyIR::emit_load_self()
{
    auto &self_val = _table.symbol(SelfObject);

    auto self = self_val._variable;
    DEBUG_ONLY(verify_oop(self));

    return self;
}

myir::oper CodeGenMyIR::emit_new_inner_helper(const std::shared_ptr<ast::Type> &klass_type, bool preserve_before_init)
{
    auto func = _runtime.symbol_by_id(RuntimeMyIR::RuntimeMyIRSymbols::GC_ALLOC)->_func;

    auto &klass = _builder->klass(klass_type->_string);

    // prepare tag, size and dispatch table
    auto tag = myir::Constant::constant(_runtime.header_elem_type(HeaderLayout::Tag), klass->tag());
    auto size = myir::Constant::constant(_runtime.header_elem_type(HeaderLayout::Size), klass->size());
    auto disp_tab = _data.class_disp_tab(klass);

    // call allocation and cast to this klass pointer
    auto object = __ call(func, {tag, size, disp_tab});

    // call init
    __ call(_module.get_function(klass->init_method()), {object});

    // object is ready
    return object;
}

myir::oper CodeGenMyIR::emit_new_inner(const std::shared_ptr<ast::Type> &klass_type)
{
    auto func = _runtime.symbol_by_id(RuntimeMyIR::RuntimeMyIRSymbols::GC_ALLOC)->_func;

    if (!semant::Semant::is_self_type(klass_type))
    {
        // in common case we need preserve object before init call
        return emit_new_inner_helper(klass_type);
    }

    auto self_val = emit_load_self();
    auto &klass = _builder->klass(_current_class->_type->_string);

    // get info about this object
    auto tag = emit_load_tag(self_val);
    auto size = emit_load_size(self_val);
    auto disp_tab = emit_load_dispatch_table(self_val);

    // save_frame();

    // allocate memory
    auto object = __ call(func, {tag, size, disp_tab});

    // lookup init method
    auto class_obj_tab = _module.get_constant(_runtime.symbol_name(RuntimeMyIR::CLASS_OBJ_TAB));

    // load init method and call
    // init method has the same type as for Object class
    auto object_init = _module.get_function(klass->init_method());

    auto init_method = __ ld<myir::POINTER>(class_obj_tab, pointer_offset(tag));

    // call this init method
    __ call(object_init, init_method, {object});

    return object;
}

myir::oper CodeGenMyIR::emit_new_expr_inner(const ast::NewExpression &expr, const std::shared_ptr<ast::Type> &expr_type)
{
    return emit_new_inner(expr._type);
}

myir::oper CodeGenMyIR::emit_load_tag(const myir::oper &obj)
{
    return __ ld<myir::UINT32>(obj, field_offset(HeaderLayoutOffsets::TagOffset));
}

myir::oper CodeGenMyIR::emit_load_size(const myir::oper &obj)
{
    return __ ld<myir::UINT64>(obj, field_offset(HeaderLayoutOffsets::SizeOffset));
}

myir::oper CodeGenMyIR::emit_load_dispatch_table(const myir::oper &obj)
{
    return __ ld<myir::POINTER>(obj, field_offset(HeaderLayoutOffsets::DispatchTableOffset));
}

myir::oper CodeGenMyIR::emit_cases_expr_inner(const ast::CaseExpression &expr,
                                              const std::shared_ptr<ast::Type> &expr_type)
{
    auto pred = emit_expr(expr._expr);

    DEBUG_ONLY(verify_oop(pred));

    // we want to generate code for the the most precise cases first, so sort cases by tag
    auto cases = expr._cases;
    std::sort(cases.begin(), cases.end(), [&](const auto &case_a, const auto &case_b) {
        return _builder->tag(case_b->_type->_string) < _builder->tag(case_a->_type->_string);
    });

    auto result = __ move(_null_val);

    auto is_not_null = __ not1(__ eq(pred, _null_val));

    myir::block true_block = nullptr, false_block = nullptr, merge_block = nullptr;
    make_control_flow(is_not_null, true_block, false_block, merge_block);

    auto tag = emit_load_tag(pred);

    // no, it is not void
    // Last case is a special case: branch to abort
    for (auto i = 0; i < cases.size(); i++)
    {
        auto &klass = _builder->klass(cases[i]->_type->_string);

        auto tag_type = _runtime.header_elem_type(HeaderLayout::Tag);

        // if object tag lower than the lowest tag for this branch, jump to next case
        auto less = __ lt(tag, myir::Constant::constant(tag_type, klass->tag()));

        // if object tag higher that the highest tag for this branch, jump to next case
        auto higher = __ gt(tag, myir::Constant::constant(tag_type, klass->child_max_tag()));

        auto need_next = __ or2(less, higher);

        auto match_block = __ new_block(Names::name(Names::FALSE_BRANCH));
        auto next_case = __ new_block(Names::name(Names::TRUE_BRANCH));

        __ cond_br(need_next, next_case, match_block);

        // match branch
        __ set_current_block(match_block);

        auto local_result = emit_in_scope(cases[i]->_object, cases[i]->_type, cases[i]->_expr, pred);
        __ move(local_result, result);
        __ br(merge_block);

        __ set_current_block(next_case);
    }

    // did not find suitable branch
    auto *case_abort = _runtime.symbol_by_id(RuntimeMyIR::RuntimeMyIRSymbols::CASE_ABORT);
    __ call(case_abort->_func, {tag});

    __ move(_null_val, result);
    __ br(merge_block);

    // pred is null
    __ set_current_block(false_block);
    auto case_abort_2_func = _runtime.symbol_by_id(RuntimeMyIR::RuntimeMyIRSymbols::CASE_ABORT_2)->_func;

    __ call(case_abort_2_func, {_data.string_const(_current_class->_file_name),
                                myir::Constant::constant(myir::INT32, expr._expr->_line_number)});
    __ move(_null_val, result);
    __ br(merge_block);

    // return value
    __ set_current_block(merge_block);
    return result;
}

myir::oper CodeGenMyIR::emit_let_expr_inner(const ast::LetExpression &expr, const std::shared_ptr<ast::Type> &expr_type)
{
    return emit_in_scope(expr._object, expr._type, expr._body_expr, expr._expr ? emit_expr(expr._expr) : nullptr);
}

myir::oper CodeGenMyIR::emit_loop_expr_inner(const ast::WhileExpression &expr,
                                             const std::shared_ptr<ast::Type> &expr_type)
{
    auto loop_header = __ new_block(Names::name(Names::LOOP_HEADER));
    auto loop_body = __ new_block(Names::name(Names::LOOP_BODY));
    auto loop_tail = __ new_block(Names::name(Names::LOOP_TAIL));

    __ br(loop_header);

    __ set_current_block(loop_header);
    __ cond_br(__ eq(emit_load_bool(emit_expr(expr._predicate)), _true_val), loop_body, loop_tail);

    __ set_current_block(loop_body);
    emit_expr(expr._body_expr);
    __ br(loop_header);

    __ set_current_block(loop_tail);

    return _null_val;
}

myir::oper CodeGenMyIR::emit_if_expr_inner(const ast::IfExpression &expr, const std::shared_ptr<ast::Type> &expr_type)
{
    auto result = __ move(_null_val);

    // do control flow
    auto pred = __ eq(emit_load_bool(emit_expr(expr._predicate)), _true_val);

    myir::block true_block = nullptr, false_block = nullptr, merge_block = nullptr;
    make_control_flow(pred, true_block, false_block, merge_block);

    // true branch
    auto true_bb_val = emit_expr(expr._true_path_expr);
    __ move(true_bb_val, result);
    __ br(merge_block);

    // false branch
    __ set_current_block(false_block);
    auto false_bb_val = emit_expr(expr._false_path_expr);
    __ move(false_bb_val, result);
    __ br(merge_block);

    // merge block
    __ set_current_block(merge_block);
    return result;
}

myir::oper CodeGenMyIR::emit_dispatch_expr_inner(const ast::DispatchExpression &expr,
                                                 const std::shared_ptr<ast::Type> &expr_type)
{
    int expr_args_size = expr._args.size();

    // prepare args
    std::vector<myir::oper> args;
    args.push_back(nullptr); // dummy for the first arg
    for (int i = 0; i < expr_args_size; i++)
    {
        args.push_back(emit_expr(expr._args.at(i)));

        DEBUG_ONLY(verify_oop(args.back()));
    }

    // get receiver
    auto receiver = emit_expr(expr._expr);
    args[0] = receiver;

    DEBUG_ONLY(verify_oop(receiver));

    auto result = __ move(_null_val);

    // check if receiver is null
    auto is_not_null = __ not1(__ eq(receiver, _null_val));

    myir::block true_block = nullptr, false_block = nullptr, merge_block = nullptr;
    make_control_flow(is_not_null, true_block, false_block, merge_block);

    auto &method_name = expr._object->_object;

    auto call = std::visit(
        ast::overloaded{
            [&](const ast::VirtualDispatchExpression &disp) {
                auto &klass =
                    _builder->klass(semant::Semant::exact_type(expr._expr->_type, _current_class->_type)->_string);

                // load dispatch table
                auto dispatch_table_ptr = emit_load_dispatch_table(receiver);

                // method has the same type as in this klass
                auto base_method = _module.get_function(klass->method_full_name(method_name));

                // load method
                auto method = __ ld<myir::POINTER>(
                    dispatch_table_ptr,
                    pointer_offset(myir::Constant::constant(myir::UINT32, klass->method_index(method_name))));

                // call
                return __ call(base_method, method, args);
            },
            [&](const ast::StaticDispatchExpression &disp) {
                auto method = _module.get_function(_builder->klass(disp._type->_string)->method_full_name(method_name));

                assert(method);

                return __ call(method, args);
            }},
        expr._base);

    __ move(call, result);
    __ br(merge_block);

    // it is null
    __ set_current_block(false_block);
    auto dispatch_abort_func = _runtime.symbol_by_id(RuntimeMyIR::RuntimeMyIRSymbols::DISPATCH_ABORT)->_func;
    __ call(dispatch_abort_func, {_data.string_const(_current_class->_file_name),
                                  myir::Constant::constant(myir::INT32, expr._expr->_line_number)});
    __ move(_null_val, result);
    __ br(merge_block);

    // merge
    __ set_current_block(merge_block);

    return result;
}

myir::oper CodeGenMyIR::emit_assign_expr_inner(const ast::AssignExpression &expr,
                                               const std::shared_ptr<ast::Type> &expr_type)
{
    auto value = emit_expr(expr._expr);
    auto &symbol = _table.symbol(expr._object->_object);

    DEBUG_ONLY(verify_oop(value));

    if (symbol._type == Symbol::LOCAL)
    {
        __ move(value, symbol._variable);
    }
    else
    {
        __ st(emit_load_self(), field_offset(symbol._offset), value);
    }

    return value;
}

myir::oper CodeGenMyIR::emit_load_int(const myir::oper &int_obj)
{
    return emit_load_primitive(int_obj);
}

myir::oper CodeGenMyIR::emit_load_primitive(const myir::oper &obj)
{
    return __ ld<myir::UINT64>(obj, field_offset(HeaderLayoutOffsets::FieldOffset));
}

myir::oper CodeGenMyIR::emit_allocate_primitive(const myir::oper &val, const std::shared_ptr<Klass> &klass)
{
    // primitive init cannot cause GC
    auto obj = emit_new_inner_helper(klass->klass(), false /* don't preserve */);

    // record value
    __ st(obj, field_offset(HeaderLayoutOffsets::FieldOffset), val);

    return obj;
}

myir::oper CodeGenMyIR::emit_allocate_int(const myir::oper &val)
{
    return emit_allocate_primitive(val, _builder->klass(BaseClassesNames[BaseClasses::INT]));
}

myir::oper CodeGenMyIR::emit_load_bool(const myir::oper &bool_obj)
{
    return emit_load_primitive(bool_obj);
}

myir::oper CodeGenMyIR::emit_in_scope(const std::shared_ptr<ast::ObjectExpression> &object,
                                      const std::shared_ptr<ast::Type> &object_type,
                                      const std::shared_ptr<ast::Expression> &expr, myir::oper initializer)
{
    _table.push_scope();

    auto local_type = semant::Semant::exact_type(object_type, _current_class->_type);

    if (!initializer)
    {
        // initial value for trivial type or null
        initializer = semant::Semant::is_trivial_type(object_type) ? _data.init_value(object_type) : _null_val;
    }

    auto variable = myir::Operand::operand(myir::POINTER, object->_object);

    _table.add_symbol(object->_object, Symbol(variable, local_type));

    __ move(initializer, variable);
    auto result = emit_expr(expr);
    DEBUG_ONLY(verify_oop(result));

    _table.pop_scope();
    return result;
}

void CodeGenMyIR::emit_runtime_main()
{
    auto runtime_main =
        std::make_shared<myir::Function>(static_cast<std::string>(RUNTIME_MAIN_FUNC),
                                         std::vector<myir::oper>{myir::Operand::operand(myir::INT32, "argc"),
                                                                 myir::Operand::operand(myir::POINTER, "argv")},
                                         myir::INT32);

    _module.add_function(runtime_main);
    __ set_current_function(runtime_main);

    auto entry = __ new_block(Names::name(Names::ENTRY_BLOCK));
    runtime_main->set_cfg(entry);

    __ set_current_block(entry);

    // first init runtime
    auto init_rt = _runtime.symbol_by_id(RuntimeMyIR::INIT_RUNTIME)->_func;
    __ call(init_rt, {runtime_main->param(0), runtime_main->param(1)});

    auto main_klass = _builder->klass(MainClassName);

    // this objects will be preserved in a callee frame
    auto main_object = emit_new_inner(main_klass->klass());

    auto main_method = main_klass->method_full_name(MainMethodName);
    __ call(_module.get_function(main_method), {main_object});

    // finish runtime
    __ call(_runtime.symbol_by_id(RuntimeMyIR::FINISH_RUNTIME)->_func, {});

    __ ret(myir::Constant::constant(myir::INT32, 0));
}

void CodeGenMyIR::emit(const std::string &out_file)
{
    std::string obj_file = out_file + static_cast<std::string>(EXT);

    emit_class_code(_builder->root()); // emit
    emit_runtime_main();

    _data.emit(obj_file);

    std::cout << _module.dump();
}