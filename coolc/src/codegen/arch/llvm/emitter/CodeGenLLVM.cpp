#include "CodeGenLLVM.h"
#include "codegen/emitter/CodeGen.inline.h"
#include "codegen/emitter/data/Data.inline.h"

using namespace codegen;

#define __ _ir_builder.

CodeGenLLVM::CodeGenLLVM(const std::shared_ptr<semant::ClassNode> &root)
    : CodeGen(std::make_shared<KlassBuilderLLVM>(root)), _ir_builder(_context),
      _module(root->_class->_file_name, _context), _runtime(_module), _data(_builder, _module, _runtime),
      _true_obj(_data.bool_const(true)), _false_obj(_data.bool_const(false)),
      _true_val(llvm::ConstantInt::get(_runtime.default_int(), TrueValue)),
      _false_val(llvm::ConstantInt::get(_runtime.default_int(), FalseValue)),
      _int0_64(llvm::ConstantInt::get(_runtime.int64_type(), 0, true)),
      _int0_32(llvm::ConstantInt::get(_runtime.int32_type(), 0, true)),
      _int0_8(llvm::ConstantInt::get(_runtime.int8_type(), 0, true)),
      _int0_8_ptr(llvm::ConstantPointerNull::get(_runtime.stack_slot_type())), _optimizer(&_module)
{
    GUARANTEE_DEBUG(_true_obj);
    GUARANTEE_DEBUG(_false_obj);

    init_optimizer();

    DEBUG_ONLY(_table.set_printer([](const std::string &name, const Symbol &s) {
        LOG("Added symbol \"" + name + "\": " + static_cast<std::string>(s))
    }));
}

void CodeGenLLVM::init_optimizer()
{
    // Do simple "peephole" optimizations and bit-twiddling optzns.
    _optimizer.add(llvm::createInstructionCombiningPass());

    // Reassociate expressions.
    _optimizer.add(llvm::createReassociatePass());

    // Eliminate Common SubExpressions.
    _optimizer.add(llvm::createGVNPass());

    // Simplify the control flow graph (deleting unreachable blocks, etc).
    _optimizer.add(llvm::createCFGSimplificationPass());

    _optimizer.doInitialization();
}

void CodeGenLLVM::add_fields()
{
    const auto &this_klass = _builder->klass(_current_class->_type->_string);

    for (auto field = this_klass->fields_begin(); field != this_klass->fields_end(); field++)
    {
        _table.add_symbol((*field)->_object->_object,
                          Symbol(this_klass->field_offset(field - this_klass->fields_begin()),
                                 semant::Semant::exact_type((*field)->_type, _current_class->_type)));
    }
}

void CodeGenLLVM::emit_class_method_inner(const std::shared_ptr<ast::Feature> &method)
{
    // it is dummies for basic classes. There are external symbols
    if (semant::Semant::is_basic_type(_current_class->_type))
    {
        return;
    }

    auto *const func = _module.getFunction(
        _builder->klass(_current_class->_type->_string)->method_full_name(method->_object->_object));

    GUARANTEE_DEBUG(func);

    func->setGC(_runtime.gc_strategy(RuntimeLLVM::SHADOW_STACK));

    // Create a new basic block to start insertion into.
    auto *entry = llvm::BasicBlock::Create(_context, Names::comment(Names::Comment::ENTRY_BLOCK), func);
    __ SetInsertPoint(entry);

    // add formals to symbol table
    // formals are local variables, so create their copy in this method and initialize by formals

    // TODO: smth better?
    _table.push_scope();
    const auto &m = std::get<ast::MethodFeature>(method->_base);
    const auto &formals = m._formals;

    std::vector<llvm::Value *> args_stack;

    _stack.resize(m._expression_stack);
    _current_stack_size = 0;
#ifdef DEBUG
    _max_stack_size = 0;
#endif // DEBUG

    // stack slots for args
    for (int i = 0; i < func->arg_size(); i++)
    {
        auto *const arg = func->getArg(i);
        args_stack.push_back(__ CreateAlloca(arg->getType()));
    }

    // stack slots for temporaries
    for (int i = 0; i < _stack.size(); i++)
    {
        _stack[i] = __ CreateAlloca(_runtime.stack_slot_type());
    }

    auto *const gcroot = llvm::Intrinsic::getDeclaration(&_module, llvm::Intrinsic::gcroot);

    // now register this slots
    for (int i = 0; i < args_stack.size(); i++)
    {
        __ CreateCall(gcroot, {__ CreateBitCast(args_stack.at(i), gcroot->getArg(0)->getType()), _int0_8_ptr});
    }

    for (int i = 0; i < _stack.size(); i++)
    {
        __ CreateCall(gcroot, {_stack[i], _int0_8_ptr});
    }

    for (auto i = 0; i < func->arg_size(); i++)
    {
        auto *const arg = func->getArg(i);
        __ CreateStore(arg, args_stack[i]);

        _table.add_symbol(static_cast<std::string>(arg->getName()),
                          Symbol(args_stack[i], i != 0 ? formals[i - 1]->_type : _current_class->_type));
    }

    // empty stack slots
    for (int i = 0; i < _stack.size(); i++)
    {
        __ CreateStore(_int0_8_ptr, _stack[i]);
    }

    __ CreateRet(maybe_cast(emit_expr(method->_expr), func->getReturnType()));

#ifdef DEBUG
    verify(func);
#endif // DEBUG

    _optimizer.run(*func);

    GUARANTEE_DEBUG(_max_stack_size == _stack.size());
    _table.pop_scope();
}

void CodeGenLLVM::preserve_value_for_gc(llvm::Value *value)
{
    __ CreateStore(__ CreateBitCast(value, _runtime.stack_slot_type()), _stack.at(_current_stack_size++));
#ifdef DEBUG
    _max_stack_size = std::max(_current_stack_size, _max_stack_size);
#endif // DEBUG
}

void CodeGenLLVM::pop_dead_value(int slots)
{
    for (int i = 0; i < slots; i++)
    {
        _current_stack_size--;
        __ CreateStore(_int0_8_ptr, _stack.at(_current_stack_size));
    }
}

llvm::Value *CodeGenLLVM::reload_value_from_stack(int slot_num, llvm::Type *type)
{
    return __ CreateLoad(type, _stack.at(slot_num));
}

llvm::Value *CodeGenLLVM::maybe_cast(llvm::Value *val, llvm::Type *type)
{
    if (val->getType() != type)
    {
        return __ CreateBitCast(val, type);
    }

    return val;
}

void CodeGenLLVM::maybe_cast(std::vector<llvm::Value *> &args, llvm::FunctionType *func)
{
    for (int i = 0; i < args.size(); i++)
    {
        args[i] = maybe_cast(args[i], func->getParamType(i));
    }
}

#ifdef DEBUG
void CodeGenLLVM::verify(llvm::Function *func)
{
    std::string error;
    llvm::raw_string_ostream llvm_error(error);
    bool has_error = llvm::verifyFunction(*func, &llvm_error);
    if (has_error)
    {
        std::cerr << error << std::endl;
        GUARANTEE_DEBUG(!has_error);
    }
}
#endif // DEBUG

void CodeGenLLVM::emit_class_init_method_inner()
{
    const auto &klass = _builder->klass(_current_class->_type->_string);

    _stack.resize(_current_class->_expression_stack);
    _current_stack_size = 0;
#ifdef DEBUG
    _max_stack_size = 0;
#endif // DEBUG

    // Note, that init method don't init header
    auto *const func = _module.getFunction(klass->init_method());

    GUARANTEE_DEBUG(func);

    func->setGC(_runtime.gc_strategy(RuntimeLLVM::SHADOW_STACK));

    // Create a new basic block to start insertion into.
    auto *entry = llvm::BasicBlock::Create(_context, Names::comment(Names::Comment::ENTRY_BLOCK), func);
    __ SetInsertPoint(entry);

    // self is visible
    _table.push_scope();

    auto *const self_formal = func->getArg(0);
    const auto &self_local = __ CreateAlloca(self_formal->getType());

    for (int i = 0; i < _stack.size(); i++)
    {
        _stack[i] = __ CreateAlloca(_runtime.stack_slot_type());
    }

    // register slots
    auto *const gcroot = llvm::Intrinsic::getDeclaration(&_module, llvm::Intrinsic::gcroot);

    __ CreateCall(gcroot, {__ CreateBitCast(self_local, gcroot->getArg(0)->getType()), _int0_8_ptr});

    for (int i = 0; i < _stack.size(); i++)
    {
        __ CreateCall(gcroot, {_stack[i], _int0_8_ptr});
    }

    __ CreateStore(self_formal, self_local);
    _table.add_symbol(SelfObject, Symbol(self_local, _current_class->_type));

    // initialize temporaries
    for (int i = 0; i < _stack.size(); i++)
    {
        __ CreateStore(_int0_8_ptr, _stack[i]);
    }

    auto *const klass_struct = _data.class_struct(klass);

    // set default value before init for fields of this class
    for (const auto &feature : _current_class->_features)
    {
        if (std::holds_alternative<ast::AttrFeature>(feature->_base))
        {
            llvm::Value *initial_val = nullptr;

            const auto &this_field = _table.symbol(feature->_object->_object);
            GUARANTEE_DEBUG(this_field._type == Symbol::FIELD); // impossible

            auto *const pointee_type = klass_struct->getTypeAtIndex(this_field._value._offset);
            // it's ok to use getArg(0) here, because safepoint is impossible
            auto *const field_ptr = __ CreateStructGEP(klass_struct, func->getArg(0), this_field._value._offset);

            if (semant::Semant::is_trivial_type(this_field._value_type))
            {
                if (semant::Semant::is_string(this_field._value_type))
                {
                    // because all strings have their unique types
                    initial_val = __ CreateBitCast(_data.init_value(this_field._value_type), pointee_type);
                }
                else
                {
                    initial_val = _data.init_value(this_field._value_type);
                }
            }
            else if (!semant::Semant::is_native_type(this_field._value_type))
            {
                GUARANTEE_DEBUG(pointee_type->isPointerTy());
                initial_val = llvm::ConstantPointerNull::get(static_cast<llvm::PointerType *>(pointee_type));
            }
            else
            {
                GUARANTEE_DEBUG(semant::Semant::is_basic_type(_current_class->_type));
                if (semant::Semant::is_native_int(this_field._value_type) ||
                    semant::Semant::is_native_bool(this_field._value_type))
                {
                    initial_val = _int0_64;
                }
                else if (semant::Semant::is_native_string(this_field._value_type))
                {
                    initial_val = _int0_8;
                }
                else
                {
                    SHOULD_NOT_REACH_HERE();
                }
            }

            __ CreateStore(initial_val, field_ptr);
        }
    }

    // call parent constructor
    if (!semant::Semant::is_empty_type(_current_class->_parent)) // Object moment
    {
        const auto parent = _builder->klass(_current_class->_parent->_string);
        auto *const parent_struct = _data.class_struct(parent);

        __ CreateCall(_module.getFunction(parent->init_method()),
                      // it's ok to use getArg(0) here, because safepoint is impossible
                      __ CreateBitCast(func->getArg(0), parent_struct->getPointerTo()));
    }

    // Now initialize
    for (const auto &feature : _current_class->_features)
    {
        if (std::holds_alternative<ast::AttrFeature>(feature->_base))
        {
            if (feature->_expr)
            {
                const auto &this_field = _table.symbol(feature->_object->_object);
                GUARANTEE_DEBUG(this_field._type == Symbol::FIELD); // impossible

                auto *const value = emit_expr(feature->_expr);

                // we have to reload self object every time because feature->_expr can contain safepoints
                auto *const field_ptr = __ CreateStructGEP(klass_struct, emit_load_self(), this_field._value._offset);
                auto *const pointee_type = klass_struct->getTypeAtIndex(this_field._value._offset);

                __ CreateStore(maybe_cast(value, pointee_type), field_ptr);
            }
        }
    }

    __ CreateRet(nullptr);

    _table.pop_scope();

#ifdef DEBUG
    verify(func);
#endif // DEBUG

    _optimizer.run(*func);

    GUARANTEE_DEBUG(_max_stack_size == _stack.size());
}

void CodeGenLLVM::make_control_flow(llvm::Value *pred, llvm::BasicBlock *&true_block, llvm::BasicBlock *&false_block,
                                    llvm::BasicBlock *&merge_block)
{
    auto *const func = __ GetInsertBlock()->getParent();

    true_block = llvm::BasicBlock::Create(_context, Names::comment(Names::Comment::TRUE_BRANCH), func);
    false_block = llvm::BasicBlock::Create(_context, Names::comment(Names::Comment::FALSE_BRANCH));
    merge_block = llvm::BasicBlock::Create(_context, Names::comment(Names::Comment::MERGE_BLOCK));

    __ CreateCondBr(pred, true_block, false_block);

    __ SetInsertPoint(true_block);
}

llvm::Value *CodeGenLLVM::emit_ternary_operator(llvm::Value *pred, llvm::Value *true_val, llvm::Value *false_val)
{
    auto *const func = __ GetInsertBlock()->getParent();

    llvm::BasicBlock *true_block = nullptr, *false_block = nullptr, *merge_block = nullptr;
    make_control_flow(pred, true_block, false_block, merge_block);

    // true block
    __ CreateBr(merge_block);

    // false block
    func->getBasicBlockList().push_back(false_block);
    __ SetInsertPoint(false_block);
    __ CreateBr(merge_block);

    // merge block
    func->getBasicBlockList().push_back(merge_block);
    __ SetInsertPoint(merge_block);
    auto *const result = __ CreatePHI(true_val->getType(), 2);

    result->addIncoming(true_val, true_block);
    result->addIncoming(false_val, false_block);

    return result;
}

llvm::Value *CodeGenLLVM::emit_binary_expr_inner(const ast::BinaryExpression &expr,
                                                 const std::shared_ptr<ast::Type> &expr_type)
{
    auto *lhs = emit_expr(expr._lhs);

    // preserve lhs on stack for gc
    preserve_value_for_gc(lhs);

    auto *const rhs = emit_expr(expr._rhs);
    lhs = reload_value_from_stack(_current_stack_size - 1, lhs->getType());

    auto logical_result = false;
    llvm::Value *op_result = nullptr;

    if (!std::holds_alternative<ast::EqExpression>(expr._base))
    {
        auto *const lv = emit_load_int(lhs); // load real values
        auto *const rv = emit_load_int(rhs);

        op_result = std::visit(
            ast::overloaded{[&](const ast::MinusExpression &minus) { return __ CreateSub(lv, rv); },
                            [&](const ast::PlusExpression &plus) { return __ CreateAdd(lv, rv); },
                            [&](const ast::DivExpression &div) { return __ CreateSDiv(lv, rv); },
                            [&](const ast::MulExpression &mul) { return __ CreateMul(lv, rv); },
                            [&](const ast::LTExpression &lt) {
                                logical_result = true;
                                return emit_ternary_operator(__ CreateICmpSLT(lv, rv), _true_obj, _false_obj);
                            },
                            [&](const ast::LEExpression &le) {
                                logical_result = true;
                                return emit_ternary_operator(__ CreateICmpSLE(lv, rv), _true_obj, _false_obj);
                            },
                            [&](const ast::EqExpression &le) { return static_cast<llvm::Value *>(nullptr); }},
            expr._base);
    }
    else
    {
        logical_result = true;

        // cast to void pointers for compare
        auto *const raw_lhs = __ CreateBitCast(lhs, _runtime.void_ptr_type());
        auto *const raw_rhs = __ CreateBitCast(rhs, _runtime.void_ptr_type());

        auto *const is_same_ref = __ CreateICmpEQ(raw_lhs, raw_rhs);

        // do control flow
        auto *const func = __ GetInsertBlock()->getParent();

        llvm::BasicBlock *true_block = nullptr, *false_block = nullptr, *merge_block = nullptr;
        make_control_flow(is_same_ref, true_block, false_block, merge_block);

        // true branch - just jump to merge
        __ SetInsertPoint(true_block);
        __ CreateBr(merge_block);

        // false branch - runtime call to equals
        func->getBasicBlockList().push_back(false_block);
        __ SetInsertPoint(false_block);

        auto *equals_func = _runtime.symbol_by_id(RuntimeLLVM::RuntimeLLVMSymbols::EQUALS)->_func;
        auto *const eq_call_res = __ CreateCall(equals_func, {raw_lhs, raw_rhs});

        auto *const false_branch_res = emit_ternary_operator(
            __ CreateICmpEQ(eq_call_res, llvm::ConstantInt::get(equals_func->getReturnType(), TrueValue, true)),
            _true_obj, _false_obj);

        false_block = __ GetInsertBlock(); // emit_ternary_operator changed cfg
        __ CreateBr(merge_block);

        // merge results
        func->getBasicBlockList().push_back(merge_block);
        __ SetInsertPoint(merge_block);
        op_result = __ CreatePHI(_true_obj->getType(), 2);
        static_cast<llvm::PHINode *>(op_result)->addIncoming(_true_obj, true_block);
        static_cast<llvm::PHINode *>(op_result)->addIncoming(false_branch_res, false_block);
    }

    pop_dead_value();

    return logical_result ? op_result : emit_allocate_int(op_result);
}

llvm::Value *CodeGenLLVM::emit_unary_expr_inner(const ast::UnaryExpression &expr,
                                                const std::shared_ptr<ast::Type> &expr_type)
{
    auto *const operand = emit_expr(expr._expr);

    return std::visit(
        ast::overloaded{
            [&](const ast::IsVoidExpression &isvoid) {
                return emit_ternary_operator(
                    __ CreateICmpEQ(
                        operand, llvm::ConstantPointerNull::get(static_cast<llvm::PointerType *>(operand->getType()))),
                    _true_obj, _false_obj);
            },
            [&](const ast::NotExpression &) {
                auto *const not_res_val = __ CreateXor(emit_load_bool(operand), _true_val);

                return emit_ternary_operator(__ CreateICmpEQ(not_res_val, _true_val), _true_obj, _false_obj);
            },
            [&](const ast::NegExpression &neg) { return emit_allocate_int(__ CreateNeg(emit_load_int(operand))); }},
        expr._base);
}

llvm::Value *CodeGenLLVM::emit_bool_expr(const ast::BoolExpression &expr, const std::shared_ptr<ast::Type> &expr_type)
{
    return _data.bool_const(expr._value);
}

llvm::Value *CodeGenLLVM::emit_int_expr(const ast::IntExpression &expr, const std::shared_ptr<ast::Type> &expr_type)
{
    return _data.int_const(expr._value);
}

llvm::Value *CodeGenLLVM::emit_string_expr(const ast::StringExpression &expr,
                                           const std::shared_ptr<ast::Type> &expr_type)
{
    static auto *const GENERIC_STRING_TYPE = _data.class_struct(_builder->klass(BaseClassesNames[BaseClasses::STRING]));
    return __ CreateBitCast(_data.string_const(expr._string), GENERIC_STRING_TYPE->getPointerTo());
}

llvm::Value *CodeGenLLVM::emit_object_expr_inner(const ast::ObjectExpression &expr,
                                                 const std::shared_ptr<ast::Type> &expr_type)
{
    const auto &object = _table.symbol(expr._object);

    auto *ptr = static_cast<llvm::Value *>(nullptr);
    auto type = std::shared_ptr<ast::Type>(nullptr);

    if (object._type == Symbol::FIELD)
    {
        const auto &klass = _builder->klass(_current_class->_type->_string);
        const auto &index = object._value._offset;

        ptr = __ CreateStructGEP(_data.class_struct(klass), emit_load_self(), index);
        type =
            semant::Semant::exact_type(static_pointer_cast<KlassLLVM>(klass)->field_type(index), _current_class->_type);
    }
    else
    {
        ptr = object._value._ptr;
        type = object._value_type;
    }

    return __ CreateLoad(_data.class_struct(_builder->klass(type->_string))->getPointerTo(), ptr);
}

llvm::Value *CodeGenLLVM::emit_load_self()
{
    const auto &self_val = _table.symbol(SelfObject);

    return __ CreateLoad(_data.class_struct(_builder->klass(self_val._value_type->_string))->getPointerTo(),
                         self_val._value._ptr);
}

llvm::Value *CodeGenLLVM::emit_new_inner_helper(const std::shared_ptr<ast::Type> &klass_type, bool preserve_before_init)
{
    auto *const func = _runtime.symbol_by_id(RuntimeLLVM::RuntimeLLVMSymbols::GC_ALLOC)->_func;

    const auto &klass = _builder->klass(klass_type->_string);

    // prepare tag, size and dispatch table
    auto *const tag = llvm::ConstantInt::get(_runtime.header_elem_type(HeaderLayout::Tag), klass->tag());
    auto *const size = llvm::ConstantInt::get(_runtime.header_elem_type(HeaderLayout::Size), klass->size());
    auto *const disp_tab = __ CreateBitCast(_data.class_disp_tab(klass), _runtime.void_ptr_type());

    // call allocation and cast to this klass pointer
    auto *const raw_object = __ CreateCall(func, {tag, size, disp_tab});
    auto *object = __ CreateBitCast(raw_object, _data.class_struct(klass)->getPointerTo());

    if (preserve_before_init)
    {
        preserve_value_for_gc(object); // init call cause GC
    }

    // call init
    __ CreateCall(_module.getFunction(klass->init_method()), {object});

    if (preserve_before_init)
    {
        object = reload_value_from_stack(_current_stack_size - 1, object->getType());
        pop_dead_value();
    }

    // object is ready
    return object;
}

llvm::Value *CodeGenLLVM::emit_new_inner(const std::shared_ptr<ast::Type> &klass_type)
{
    auto *const func = _runtime.symbol_by_id(RuntimeLLVM::RuntimeLLVMSymbols::GC_ALLOC)->_func;

    if (!semant::Semant::is_self_type(klass_type))
    {
        // in common case we need preserve object before init call
        return emit_new_inner_helper(klass_type);
    }

    auto *const self_val = emit_load_self();
    const auto &klass = _builder->klass(_current_class->_type->_string);
    const auto &klass_struct = _data.class_struct(klass);

    // get info about this object
    auto *const tag = emit_load_tag(self_val, klass_struct);
    auto *const size = emit_load_size(self_val, klass_struct);
    auto *const disp_tab = __ CreateBitCast(emit_load_dispatch_table(self_val, klass), _runtime.void_ptr_type());

    // allocate memory
    llvm::Value *raw_object = __ CreateCall(func, {tag, size, disp_tab});

    preserve_value_for_gc(raw_object); // init call cause GC

    // lookup init method
    auto *const class_obj_tab =
        _module.getNamedGlobal(_runtime.symbol_name(RuntimeLLVM::RuntimeLLVMSymbols::CLASS_OBJ_TAB));

    auto *const init_method_ptr = __ CreateGEP(class_obj_tab->getValueType(), class_obj_tab, {_int0_64, tag});

    // load init method and call
    // init method has the same type as for Object class
    auto *const object_init = _module.getFunction(klass->init_method());
    auto *const init_method = __ CreateLoad(object_init->getType(), init_method_ptr);

    // call this init method
    __ CreateCall(object_init->getFunctionType(), init_method,
                  {maybe_cast(raw_object, object_init->getArg(0)->getType())});

    raw_object = reload_value_from_stack(_current_stack_size - 1, raw_object->getType());

    pop_dead_value();
    return raw_object;
}

llvm::Value *CodeGenLLVM::emit_new_expr_inner(const ast::NewExpression &expr,
                                              const std::shared_ptr<ast::Type> &expr_type)
{
    return emit_new_inner(expr._type);
}

llvm::Value *CodeGenLLVM::emit_load_tag(llvm::Value *obj, llvm::Type *obj_type)
{
    auto *const tag_ptr = __ CreateStructGEP(obj_type, obj, HeaderLayout::Tag);

    return __ CreateLoad(_runtime.header_elem_type(HeaderLayout::Tag), tag_ptr);
}

llvm::Value *CodeGenLLVM::emit_load_size(llvm::Value *obj, llvm::Type *obj_type)
{
    auto *const size_ptr = __ CreateStructGEP(obj_type, obj, HeaderLayout::Size);

    return __ CreateLoad(_runtime.header_elem_type(HeaderLayout::Size), size_ptr);
}

llvm::Value *CodeGenLLVM::emit_load_dispatch_table(llvm::Value *obj, const std::shared_ptr<Klass> &klass)
{
    auto *const dispatch_table_ptr_ptr =
        __ CreateStructGEP(obj->getType()->getPointerElementType(), obj, HeaderLayout::DispatchTable);

    return __ CreateLoad(_data.class_disp_tab(klass)->getType(), dispatch_table_ptr_ptr);
}

llvm::Value *CodeGenLLVM::emit_cases_expr_inner(const ast::CaseExpression &expr,
                                                const std::shared_ptr<ast::Type> &expr_type)
{
    // TODO: maybe use SwitchInst?
    auto *const pred = emit_expr(expr._expr);

    // we want to generate code for the the most precise cases first, so sort cases by tag
    auto cases = expr._cases;
    std::sort(cases.begin(), cases.end(), [&](const auto &case_a, const auto &case_b) {
        return _builder->tag(case_b->_type->_string) < _builder->tag(case_a->_type->_string);
    });

    // save results and blocks for phi
    std::vector<std::pair<llvm::BasicBlock *, llvm::Value *>> results;

    auto *const func = __ GetInsertBlock()->getParent();

    auto *const is_not_null = __ CreateIsNotNull(pred);

    llvm::BasicBlock *true_block = nullptr, *false_block = nullptr, *merge_block = nullptr;
    make_control_flow(is_not_null, true_block, false_block, merge_block);

    auto *const tag =
        emit_load_tag(pred, _data.class_struct(_builder->klass(
                                semant::Semant::exact_type(expr._expr->_type, _current_class->_type)->_string)));

    auto *const res_ptr_type =
        _data.class_struct(_builder->klass(semant::Semant::exact_type(expr_type, _current_class->_type)->_string))
            ->getPointerTo();

    // no, it is not void
    // Last case is a special case: branch to abort
    for (auto i = 0; i < cases.size(); i++)
    {
        const auto &klass = _builder->klass(cases[i]->_type->_string);

        auto *const tag_type = _runtime.header_elem_type(HeaderLayout::Tag);

        // if object tag lower than the lowest tag for this branch, jump to next case
        auto *const less = __ CreateICmpSLT(tag, llvm::ConstantInt::get(tag_type, klass->tag()));

        // if object tag higher that the highest tag for this branch, jump to next case
        auto *const higher = __ CreateICmpSGT(tag, llvm::ConstantInt::get(tag_type, klass->child_max_tag()));

        auto *const need_next = __ CreateOr(less, higher);

        auto *match_block = llvm::BasicBlock::Create(_context, Names::comment(Names::Comment::FALSE_BRANCH));
        auto *const next_case = llvm::BasicBlock::Create(_context, Names::comment(Names::Comment::TRUE_BRANCH));

        // TODO: set weight?
        __ CreateCondBr(need_next, next_case, match_block);

        func->getBasicBlockList().push_back(match_block);
        __ SetInsertPoint(match_block);

        // match branch
        auto *const result = emit_in_scope(cases[i]->_object, cases[i]->_type, cases[i]->_expr, pred);
        auto *const casted_res = __ CreateBitCast(result, res_ptr_type);
        match_block = __ GetInsertBlock();
        results.push_back({match_block, casted_res});

        __ CreateBr(merge_block);

        func->getBasicBlockList().push_back(next_case);
        __ SetInsertPoint(next_case);
    }

    auto *const null_result = llvm::ConstantPointerNull::get(res_ptr_type);

    // did not find suitable branch
    auto *const case_abort = _runtime.symbol_by_id(RuntimeLLVM::RuntimeLLVMSymbols::CASE_ABORT);
    __ CreateCall(case_abort->_func->getFunctionType(), case_abort->_func, {tag});

    // do it just for phi
    results.push_back({__ GetInsertBlock(), null_result});
    __ CreateBr(merge_block);

    // pred is null
    func->getBasicBlockList().push_back(false_block);
    __ SetInsertPoint(false_block);
    auto *const case_abort_2_func = _runtime.symbol_by_id(RuntimeLLVM::RuntimeLLVMSymbols::CASE_ABORT_2)->_func;
    __ CreateCall(case_abort_2_func,
                  {__ CreateBitCast(_data.string_const(_current_class->_file_name), _runtime.void_ptr_type()),
                   llvm::ConstantInt::get(_runtime.int32_type(), expr._expr->_line_number)});
    __ CreateBr(merge_block);
    results.push_back({false_block, null_result});

    // return value
    func->getBasicBlockList().push_back(merge_block);
    __ SetInsertPoint(merge_block);

    auto *const result = __ CreatePHI(res_ptr_type, results.size());
    for (const auto &res : results)
    {
        result->addIncoming(res.second, res.first);
    }

    return result;
}

llvm::Value *CodeGenLLVM::emit_let_expr_inner(const ast::LetExpression &expr,
                                              const std::shared_ptr<ast::Type> &expr_type)
{
    return emit_in_scope(expr._object, expr._type, expr._body_expr, expr._expr ? emit_expr(expr._expr) : nullptr);
}

llvm::Value *CodeGenLLVM::emit_loop_expr_inner(const ast::WhileExpression &expr,
                                               const std::shared_ptr<ast::Type> &expr_type)
{
    auto *const func = __ GetInsertBlock()->getParent();

    auto *loop_header = llvm::BasicBlock::Create(_context, Names::comment(Names::Comment::LOOP_HEADER), func);
    auto *loop_body = llvm::BasicBlock::Create(_context, Names::comment(Names::Comment::LOOP_BODY));
    auto *loop_tail = llvm::BasicBlock::Create(_context, Names::comment(Names::Comment::LOOP_TAIL));

    __ CreateBr(loop_header);

    __ SetInsertPoint(loop_header);
    __ CreateCondBr(__ CreateICmpEQ(emit_load_bool(emit_expr(expr._predicate)), _true_val), loop_body, loop_tail);
    auto *const new_loop_header = __ GetInsertBlock();

    func->getBasicBlockList().push_back(loop_body);
    __ SetInsertPoint(loop_body);
    emit_expr(expr._body_expr);
    __ CreateBr(loop_header);
    loop_body = __ GetInsertBlock();

    func->getBasicBlockList().push_back(loop_tail);
    __ SetInsertPoint(loop_tail);

    return llvm::ConstantPointerNull::get(_data.class_struct(_builder->klass(expr_type->_string))->getPointerTo());
}

llvm::Value *CodeGenLLVM::emit_if_expr_inner(const ast::IfExpression &expr, const std::shared_ptr<ast::Type> &expr_type)
{
    // do control flow
    auto *const func = __ GetInsertBlock()->getParent();

    auto *const phi_type =
        _data.class_struct(_builder->klass(semant::Semant::exact_type(expr_type, _current_class->_type)->_string))
            ->getPointerTo();

    auto *const pred = __ CreateICmpEQ(emit_load_bool(emit_expr(expr._predicate)), _true_val);

    llvm::BasicBlock *true_block = nullptr, *false_block = nullptr, *merge_block = nullptr;
    make_control_flow(pred, true_block, false_block, merge_block);

    // true branch
    auto *const true_bb_val = __ CreateBitCast(emit_expr(expr._true_path_expr), phi_type);
    __ CreateBr(merge_block);
    true_block = __ GetInsertBlock(); // emit_expr can change cfg

    // false branch
    func->getBasicBlockList().push_back(false_block);
    __ SetInsertPoint(false_block);
    auto *const false_bb_val = __ CreateBitCast(emit_expr(expr._false_path_expr), phi_type);
    __ CreateBr(merge_block);
    false_block = __ GetInsertBlock(); // emit_expr can change cfg

    // merge block
    func->getBasicBlockList().push_back(merge_block);
    __ SetInsertPoint(merge_block);

    auto *const phi = __ CreatePHI(phi_type, 2);

    phi->addIncoming(true_bb_val, true_block);
    phi->addIncoming(false_bb_val, false_block);

    return phi;
}

llvm::Value *CodeGenLLVM::emit_dispatch_expr_inner(const ast::DispatchExpression &expr,
                                                   const std::shared_ptr<ast::Type> &expr_type)
{
    auto *const func = __ GetInsertBlock()->getParent();

    // prepare args
    std::vector<llvm::Value *> args;
    args.push_back(nullptr); // dummy for the first arg
    for (const auto &arg : expr._args)
    {
        args.push_back(emit_expr(arg));

        // preserve args for GC
        preserve_value_for_gc(args.back());
    }

    // get receiver
    auto *const receiver = emit_expr(expr._expr);
    args[0] = receiver;

    // reload args after possible relocation
    int slots_num = expr._args.size();
    for (int i = 1; i < args.size(); i++)
    {
        args[i] = reload_value_from_stack(_current_stack_size - (slots_num - i + 1), args.at(i)->getType());
    }

    // pop stack slots now to reduce roots number for mark phase
    pop_dead_value(slots_num);

    auto *const phi_type =
        _data.class_struct(_builder->klass(semant::Semant::exact_type(expr_type, _current_class->_type)->_string))
            ->getPointerTo();

    // check if receiver is null
    auto *const is_not_null = __ CreateIsNotNull(receiver);

    llvm::BasicBlock *true_block = nullptr, *false_block = nullptr, *merge_block = nullptr;
    make_control_flow(is_not_null, true_block, false_block, merge_block);

    const auto &method_name = expr._object->_object;
    auto *const call = std::visit(
        ast::overloaded{
            [&](const ast::VirtualDispatchExpression &disp) {
                const auto &klass =
                    _builder->klass(semant::Semant::exact_type(expr._expr->_type, _current_class->_type)->_string);

                auto *const dispatch_table_ptr = emit_load_dispatch_table(receiver, klass);

                // get pointer on method address
                // method has the same type as in this klass
                auto *const base_method = _module.getFunction(klass->method_full_name(method_name));
                auto *const method_ptr = __ CreateStructGEP(_data.class_disp_tab(klass)->getValueType(),
                                                            dispatch_table_ptr, klass->method_index(method_name));

                // load method
                auto *const method = __ CreateLoad(base_method->getType(), method_ptr);

                maybe_cast(args, base_method->getFunctionType());

                // call
                return __ CreateCall(base_method->getFunctionType(), method, args);
            },
            [&](const ast::StaticDispatchExpression &disp) {
                auto *const method =
                    _module.getFunction(_builder->klass(disp._type->_string)->method_full_name(method_name));

                GUARANTEE_DEBUG(method);

                maybe_cast(args, method->getFunctionType());

                return __ CreateCall(method, args);
            }},
        expr._base);
    auto *const casted_call = __ CreateBitCast(call, phi_type);
    __ CreateBr(merge_block);

    // it is null
    func->getBasicBlockList().push_back(false_block);
    __ SetInsertPoint(false_block);
    auto *const dispatch_abort_func = _runtime.symbol_by_id(RuntimeLLVM::RuntimeLLVMSymbols::DISPATCH_ABORT)->_func;
    __ CreateCall(dispatch_abort_func,
                  {__ CreateBitCast(_data.string_const(_current_class->_file_name), _runtime.void_ptr_type()),
                   llvm::ConstantInt::get(_runtime.int32_type(), expr._expr->_line_number)});
    __ CreateBr(merge_block);

    // merge
    func->getBasicBlockList().push_back(merge_block);
    __ SetInsertPoint(merge_block);

    auto *const phi = __ CreatePHI(phi_type, 2);

    phi->addIncoming(casted_call, true_block);
    phi->addIncoming(llvm::ConstantPointerNull::get(phi_type), false_block);

    return phi;
}

llvm::Value *CodeGenLLVM::emit_assign_expr_inner(const ast::AssignExpression &expr,
                                                 const std::shared_ptr<ast::Type> &expr_type)
{
    auto *const value = emit_expr(expr._expr);
    const auto &symbol = _table.symbol(expr._object->_object);

    llvm::Type *cast_type = nullptr;
    llvm::Value *store_dst = nullptr;
    if (symbol._type == Symbol::LOCAL)
    {
        // all locals except arguments are int8*, so cast value to it
        if (symbol._value._ptr->getType() == _runtime.stack_slot_type()->getPointerTo())
        {
            cast_type = _runtime.stack_slot_type();
        }
        else
        {
            cast_type = _data.class_struct(_builder->klass(symbol._value_type->_string))->getPointerTo();
        }
        store_dst = symbol._value._ptr;
    }
    else
    {
        auto *const klass_struct = _data.class_struct(_builder->klass(_current_class->_type->_string));

        cast_type = klass_struct->getElementType(symbol._value._offset);
        store_dst = __ CreateStructGEP(klass_struct, emit_load_self(), symbol._value._offset);
    }

    __ CreateStore(maybe_cast(value, cast_type), store_dst);

    return value;
}

llvm::Value *CodeGenLLVM::emit_load_int(llvm::Value *int_obj)
{
    return emit_load_primitive(int_obj, _data.class_struct(_builder->klass(BaseClassesNames[BaseClasses::INT])));
}

llvm::Value *CodeGenLLVM::emit_load_primitive(llvm::Value *obj, llvm::Type *obj_type)
{
    const auto &value_ptr = __ CreateStructGEP(obj_type, obj, HeaderLayout::DispatchTable + 1);

    return __ CreateLoad(static_cast<llvm::StructType *>(obj_type)->getElementType(HeaderLayout::DispatchTable + 1),
                         value_ptr);
}

llvm::Value *CodeGenLLVM::emit_allocate_primitive(llvm::Value *val, const std::shared_ptr<Klass> &klass)
{
    // allocate
    // primitive init cannot cause GC
    auto *const obj = emit_new_inner_helper(klass->klass(), false /* don't preserve */);

    // record value
    auto *const val_ptr = __ CreateStructGEP(_data.class_struct(klass), obj, HeaderLayout::DispatchTable + 1);
    __ CreateStore(val, val_ptr);

    return obj;
}

llvm::Value *CodeGenLLVM::emit_allocate_int(llvm::Value *val)
{
    return emit_allocate_primitive(val, _builder->klass(BaseClassesNames[BaseClasses::INT]));
}

llvm::Value *CodeGenLLVM::emit_load_bool(llvm::Value *bool_obj)
{
    return emit_load_primitive(bool_obj, _data.class_struct(_builder->klass(BaseClassesNames[BaseClasses::BOOL])));
}

llvm::Value *CodeGenLLVM::emit_in_scope(const std::shared_ptr<ast::ObjectExpression> &object,
                                        const std::shared_ptr<ast::Type> &object_type,
                                        const std::shared_ptr<ast::Expression> &expr, llvm::Value *initializer)
{
    _table.push_scope();

    const auto local_type = semant::Semant::exact_type(object_type, _current_class->_type);
    auto *const object_ptr_type = _data.class_struct(_builder->klass(local_type->_string))->getPointerTo();

    if (!initializer)
    {
        // initial value for trivial type or null
        initializer = semant::Semant::is_trivial_type(object_type)
                          ? static_cast<llvm::Value *>(_data.init_value(object_type))
                          : static_cast<llvm::Value *>(llvm::ConstantPointerNull::get(object_ptr_type));
    }

    preserve_value_for_gc(initializer);

    // allocate pointer to local variable
    auto *const local_val = _stack[_current_stack_size - 1];
    _table.add_symbol(object->_object, Symbol(local_val, local_type));

    auto *const result = emit_expr(expr);
    _table.pop_scope();

    pop_dead_value();

    return result;
}

void CodeGenLLVM::emit_runtime_main()
{
    const auto &runtime_main =
        llvm::Function::Create(llvm::FunctionType::get(llvm::Type::getInt32Ty(_context),
                                                       {
                                                           _runtime.int32_type(),                     // int argc
                                                           _runtime.stack_slot_type()->getPointerTo() // char** argv
                                                       },
                                                       false),
                               llvm::Function::ExternalLinkage, static_cast<std::string>(RUNTIME_MAIN_FUNC), &_module);
    runtime_main->setGC(_runtime.gc_strategy(RuntimeLLVM::SHADOW_STACK));

    auto *entry = llvm::BasicBlock::Create(_context, Names::comment(Names::Comment::ENTRY_BLOCK), runtime_main);
    __ SetInsertPoint(entry);

    auto *const gcroot = llvm::Intrinsic::getDeclaration(&_module, llvm::Intrinsic::gcroot);

    // need at least one slot for main object
    _stack.resize(1);
    _stack[0] = __ CreateAlloca(_runtime.stack_slot_type());
    __ CreateCall(gcroot, {_stack.at(0), _int0_8_ptr});
    _current_stack_size = 0;

    // first init runtime
    auto *const init_rt = _runtime.symbol_by_id(RuntimeLLVM::INIT_RUNTIME)->_func;
    // this objects will be preserved in a callee frame
    __ CreateCall(init_rt, {runtime_main->getArg(0), runtime_main->getArg(1)});

    const auto main_klass = _builder->klass(MainClassName);
    auto *const main_object = emit_new_inner(main_klass->klass());

    const auto main_method = main_klass->method_full_name(MainMethodName);
    __ CreateCall(_module.getFunction(main_method), {main_object});

    // finish runtime
    __ CreateCall(_runtime.symbol_by_id(RuntimeLLVM::FINISH_RUNTIME)->_func);

    __ CreateRet(_int0_32);

#ifdef DEBUG
    verify(runtime_main);
#endif // DEBUG

    _optimizer.run(*runtime_main);
}

#define EXIT_ON_ERROR(cond, error)                                                                                     \
    if (!cond)                                                                                                         \
    {                                                                                                                  \
        std::cerr << error << std::endl;                                                                               \
        exit(-1);                                                                                                      \
    }

void CodeGenLLVM::execute_linker(const std::string &object_file_name, const std::string &out_file_name)
{
    CODEGEN_VERBOSE_ONLY(LOG("Run linker for " + object_file_name + "."));

    const auto coolc_path = boost::dll::program_location().parent_path().string();
    const auto rt_lib_path =
        coolc_path + boost::filesystem::path::preferred_separator + static_cast<std::string>(RUNTIME_LIB_NAME);
    CODEGEN_VERBOSE_ONLY(LOG("Runtime library path: " + rt_lib_path));

    const auto clang_path = llvm::sys::findProgramByName(static_cast<std::string>(CLANG_EXE_NAME));
    EXIT_ON_ERROR(clang_path, "Can't find " + static_cast<std::string>(CLANG_EXE_NAME));
    CODEGEN_VERBOSE_ONLY(LOG(static_cast<std::string>(CLANG_EXE_NAME) + " library path: " + clang_path.get()));

    std::string error;
    // create executable
    EXIT_ON_ERROR((llvm::sys::ExecuteAndWait(clang_path.get(),
                                             {clang_path.get(), object_file_name, rt_lib_path,
#ifdef ASAN
                                              "-fsanitize=address", "-fno-omit-frame-pointer",
#elif UBSAN
                                              "-fsanitize=undefined", "-fno-omit-frame-pointer",
#endif // UBSAN

                                              "-o", out_file_name}, // first arg is file name
                                             llvm::None, {}, 0, 0, &error) == 0),
                  error);

    // delete object file
    std::filesystem::remove(object_file_name);

    CODEGEN_VERBOSE_ONLY(LOG("Finish linker for " + out_file_name + "."));
}

std::pair<std::string, std::string> CodeGenLLVM::find_best_vec_ext()
{
    if (!UseArchSpecFeatures)
    {
        return {"", "generic"};
    }

    const auto cpu = static_cast<std::string>(llvm::sys::getHostCPUName());

    llvm::StringMap<bool> features;
    llvm::sys::getHostCPUFeatures(features);

    const auto vec_ext_list =
#ifdef __x86_64__
        {"avx512f", "avx512vl", "avx2", "avx", "sse4.2", "sse4.1", "sse3", "sse2", "sse"};
#elif __aarch64__
        {""};
#else
        {""};
#endif

    for (const auto &ext : vec_ext_list)
    {
        const auto elem = features.find(ext);
        if (elem != features.end() && elem->getValue())
        {
            return {static_cast<std::string>(elem->getKey()), cpu};
        }
    }

    return {"", cpu};
}

void CodeGenLLVM::emit(const std::string &out_file)
{
    const std::string obj_file = out_file + static_cast<std::string>(EXT);

    _data.emit(obj_file);

    emit_class_code(_builder->root()); // emit
    emit_runtime_main();

    CODEGEN_VERBOSE_ONLY(_module.print(llvm::errs(), nullptr););

    const auto target_triple = llvm::sys::getDefaultTargetTriple();
    CODEGEN_VERBOSE_ONLY(LOG("Target arch: " + target_triple));

    const auto arch_spec = find_best_vec_ext();

    CODEGEN_VERBOSE_ONLY(LOG("Target CPU: " + arch_spec.second));
    CODEGEN_VERBOSE_ONLY(LOG("Target Features: " + arch_spec.first));

    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();

    CODEGEN_VERBOSE_ONLY(LOG("Initialized llvm emitter."));

    std::string error;
    const auto *const target = llvm::TargetRegistry::lookupTarget(target_triple, error);
    EXIT_ON_ERROR(target, error);

    CODEGEN_VERBOSE_ONLY(LOG("Found target: " + std::string(target->getName())));

    auto *const target_machine = target->createTargetMachine(
        target_triple, arch_spec.second, arch_spec.first, llvm::TargetOptions(), llvm::Optional<llvm::Reloc::Model>());
    EXIT_ON_ERROR(target_machine, "Can't create target machine!");

    _module.setDataLayout(target_machine->createDataLayout());
    _module.setTargetTriple(target_triple);

    CODEGEN_VERBOSE_ONLY(LOG("Initialized target machine."));

    // open object file
    std::error_code ec;
    llvm::raw_fd_ostream dest(obj_file, ec);
    EXIT_ON_ERROR(!ec, "Could not open file: " + ec.message());

    llvm::legacy::PassManager pass;
    EXIT_ON_ERROR(!target_machine->addPassesToEmitFile(pass, dest, nullptr, llvm::CGFT_ObjectFile),
                  "TargetMachine can't emit a file of this type!");

    CODEGEN_VERBOSE_ONLY(LOG("Run llvm emitter."));

    pass.run(_module);
    dest.flush();
    dest.close();

    CODEGEN_VERBOSE_ONLY(LOG("Finished llvm emitter."));

    execute_linker(obj_file, out_file);

    delete target_machine;
}