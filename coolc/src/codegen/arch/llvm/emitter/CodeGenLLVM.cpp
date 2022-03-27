#include "CodeGenLLVM.h"
#include "codegen/emitter/CodeGen.inline.h"
#include "codegen/emitter/data/Data.inline.h"

using namespace codegen;

#define __ _ir_builder.

// TODO: good module name?
// TODO: module name to constant?
CodeGenLLVM::CodeGenLLVM(const std::shared_ptr<semant::ClassNode> &root)
    : CodeGen(std::make_shared<KlassBuilderLLVM>(root)), _ir_builder(_context), _module("MainModule", _context),
      _runtime(_module), _data(_builder, _module, _runtime)
{
    DEBUG_ONLY(_table.set_printer([](const std::string &name, const Symbol &s) {
        LOG("Add symbol " + name + " is " + static_cast<std::string>(s))
    }));
}

void CodeGenLLVM::add_fields()
{
    const auto &this_klass = _builder->klass(_current_class->_type->_string);
    for (auto field = this_klass->fields_begin(); field != this_klass->fields_end(); field++)
    {
        const std::string &name = (*field)->_object->_object;
        _table.add_symbol(name, Symbol(this_klass->field_offset(field - this_klass->fields_begin())));
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

    // add formals to symbol table
    _table.push_scope();
    for (auto &arg : func->args())
    {
        _table.add_symbol(arg.getName(), &arg);
    }

    // Create a new basic block to start insertion into.
    auto *entry = llvm::BasicBlock::Create(_context, static_cast<std::string>(NameConstructor::ENTRY_BLOCK_NAME), func);
    __ SetInsertPoint(entry);

    __ CreateRet(emit_expr(method->_expr));

    // TODO: verifier don't compile
    // GUARANTEE_DEBUG(!llvm::verifyFunction(*func));

    _table.pop_scope();
}

void CodeGenLLVM::emit_class_init_method_inner()
{
    const auto &klass = _builder->klass(_current_class->_type->_string);
    auto *const func = _module.getFunction(NameConstructor::init_method(klass));

    GUARANTEE_DEBUG(func);

    // Create a new basic block to start insertion into.
    auto *entry = llvm::BasicBlock::Create(_context, static_cast<std::string>(NameConstructor::ENTRY_BLOCK_NAME), func);
    __ SetInsertPoint(entry);

    // call parent constructor
    if (!semant::Semant::is_empty_type(_current_class->_parent)) // Object moment
    {
        const auto &parent_init = NameConstructor::init_method(_builder->klass(_current_class->_parent->_string));
        // TODO: maybe better way to pass args?
        __ CreateCall(_module.getFunction(parent_init), {func->getArg(0), func->getArg(1), func->getArg(2)},
                      NameConstructor::call(parent_init));
    }

    for (const auto &feature : _current_class->_features)
    {
        if (std::holds_alternative<ast::AttrFeature>(feature->_base))
        {
            if (feature->_expr)
            {
                const std::string &name = feature->_object->_object;

                const auto &this_field = _table.symbol(name);
                GUARANTEE_DEBUG(this_field._type == Symbol::FIELD); // impossible

                const int &offset = this_field._value._offset;
                // TODO: check this inst twice
                __ CreateStore(emit_expr(feature->_expr), __ CreateStructGEP(_data.class_struct(klass), func->getArg(0),
                                                                             offset, NameConstructor::gep(name)));
            }
        }
    }

    // TODO: is it correct?
    __ CreateRet(nullptr);

    // TODO: verifier don't compile
    // GUARANTEE_DEBUG(!llvm::verifyFunction(*func));
}

// TODO: all temporaries names to constants?
llvm::Value *CodeGenLLVM::emit_binary_expr_inner(const ast::BinaryExpression &expr)
{
    auto *const lhs = emit_expr(expr._lhs);
    auto *const rhs = emit_expr(expr._rhs);

    bool logical_result = false;
    llvm::Value *op_result = nullptr;

    if (!std::holds_alternative<ast::EqExpression>(expr._base))
    {
        auto *const lv = emit_load_int(lhs); // load real values
        auto *const rv = emit_load_int(rhs);

        op_result = std::visit(
            ast::overloaded{
                [&](const ast::MinusExpression &minus) -> llvm::Value * { return __ CreateSub(lv, rv, "sub_tmp"); },
                [&](const ast::PlusExpression &plus) -> llvm::Value * { return __ CreateAdd(lv, rv, "add_tmp"); },
                [&](const ast::DivExpression &div) -> llvm::Value * {
                    return __ CreateSDiv(lv, rv, "div_tmp"); /* TODO: SDiv? */
                },
                [&](const ast::MulExpression &mul) -> llvm::Value * { return __ CreateMul(lv, rv, "mul_tmp"); },
                [&](const ast::LTExpression &lt) -> llvm::Value * {
                    logical_result = true;
                    return __ CreateICmpSLT(lv, rv, "sl_tmp");
                },
                [&](const ast::LEExpression &le) -> llvm::Value * {
                    // TODO: check this twice!
                    logical_result = true;
                    return __ CreateICmpSLE(lv, rv, "le_tmp");
                },
                [&](const ast::EqExpression &le) -> llvm::Value * { return nullptr; }},
            expr._base);
    }
    else
    {
        logical_result = true;

        auto *const is_same_ref = __ CreateICmpEQ(lhs, rhs);

        // do control flow
        auto *const func = __ GetInsertBlock()->getParent();
        auto *const true_bb = llvm::BasicBlock::Create(_context, "eq_ref_check_true", func);
        auto *const false_bb = llvm::BasicBlock::Create(_context, "eq_ref_check_false");
        auto *const merge_bb = llvm::BasicBlock::Create(_context, "eq_ref_check_cont");

        __ CreateCondBr(is_same_ref, true_bb, false_bb);

        // true branch - just jump to merge
        __ SetInsertPoint(true_bb);
        __ CreateBr(merge_bb);

        // false branch - runtime call to equals
        func->getBasicBlockList().push_back(false_bb);
        __ SetInsertPoint(false_bb);

        GUARANTEE_DEBUG(_runtime.method(RuntimeMethodsNames[RuntimeMethods::EQUALS]));
        auto *equals_func = _runtime.method(RuntimeMethodsNames[RuntimeMethods::EQUALS])->_func;

        auto *const false_branch_res =
            __ CreateCall(equals_func, {lhs, rhs}, NameConstructor::call(RuntimeMethodsNames[RuntimeMethods::EQUALS]));
        __ CreateBr(merge_bb);

        // merge results
        func->getBasicBlockList().push_back(merge_bb);
        __ SetInsertPoint(merge_bb);
        auto *const res_type = equals_func->getReturnType();
        op_result = __ CreatePHI(res_type, 2, "eq_tmp");
        ((llvm::PHINode *)op_result)->addIncoming(llvm::ConstantInt::get(res_type, 1, true), true_bb);
        ((llvm::PHINode *)op_result)->addIncoming(false_branch_res, false_bb);
    }

    return logical_result ? emit_allocate_bool(op_result) : emit_allocate_int(op_result);
}

llvm::Value *CodeGenLLVM::emit_unary_expr_inner(const ast::UnaryExpression &expr)
{
}

llvm::Value *CodeGenLLVM::emit_bool_expr(const ast::BoolExpression &expr)
{
}

llvm::Value *CodeGenLLVM::emit_int_expr(const ast::IntExpression &expr)
{
    return _data.int_const(expr._value);
}

llvm::Value *CodeGenLLVM::emit_string_expr(const ast::StringExpression &expr)
{
}

llvm::Value *CodeGenLLVM::emit_object_expr_inner(const ast::ObjectExpression &expr)
{
}

llvm::Value *CodeGenLLVM::emit_new_expr_inner(const ast::NewExpression &expr)
{
}

llvm::Value *CodeGenLLVM::emit_cases_expr_inner(const ast::CaseExpression &expr)
{
}

llvm::Value *CodeGenLLVM::emit_let_expr_inner(const ast::LetExpression &expr)
{
}

llvm::Value *CodeGenLLVM::emit_loop_expr_inner(const ast::WhileExpression &expr)
{
}

llvm::Value *CodeGenLLVM::emit_if_expr_inner(const ast::IfExpression &expr)
{
}

llvm::Value *CodeGenLLVM::emit_dispatch_expr_inner(const ast::DispatchExpression &expr)
{
}

llvm::Value *CodeGenLLVM::emit_assign_expr_inner(const ast::AssignExpression &expr)
{
}

llvm::Value *CodeGenLLVM::emit_load_int(const llvm::Value *int_obj)
{
}

llvm::Value *CodeGenLLVM::emit_allocate_int(const llvm::Value *val)
{
}

llvm::Value *CodeGenLLVM::emit_load_bool(const llvm::Value *bool_obj)
{
}

llvm::Value *CodeGenLLVM::emit_allocate_bool(const llvm::Value *val)
{
}

void CodeGenLLVM::emit_runtime_main()
{
    const auto &runtime_main =
        llvm::Function::Create(llvm::FunctionType::get(llvm::Type::getInt32Ty(_context), {}, false),
                               llvm::Function::ExternalLinkage, static_cast<std::string>(RUNTIME_MAIN_FUNC), &_module);

    auto *entry =
        llvm::BasicBlock::Create(_context, static_cast<std::string>(NameConstructor::ENTRY_BLOCK_NAME), runtime_main);
    __ SetInsertPoint(entry);

    // TODO: dummy
    __ CreateCall(_runtime.method(RuntimeMethodsNames[RuntimeMethods::GC_ALLOC])->_func,
                  {llvm::ConstantInt::get(llvm::Type::getInt64Ty(_context), 0)});

    __ CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(_context), 0));

    // TODO: verifier don't compile
    // GUARANTEE_DEBUG(!llvm::verifyFunction(*runtime_main));
}

void CodeGenLLVM::emit(const std::string &out_file)
{
    // emit_class_code(_builder->root()); // emit
    emit_runtime_main();

    CODEGEN_VERBOSE_ONLY(_module.print(llvm::errs(), nullptr););

    const auto target_triple = llvm::sys::getDefaultTargetTriple();
    CODEGEN_VERBOSE_ONLY(LOG("Target architecture: " + target_triple));

    // TODO: what we really need?
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();

    std::string error;
    const auto *const target = llvm::TargetRegistry::lookupTarget(target_triple, error);
    // TODO: better way for error handling
    if (!target)
    {
        std::cerr << error << std::endl;
        exit(-1);
    }

    // TODO: opportunity to select options
    auto *const target_machine = target->createTargetMachine(target_triple, "generic", "", llvm::TargetOptions(),
                                                             llvm::Optional<llvm::Reloc::Model>());

    // TODO: any settings?
    _module.setDataLayout(target_machine->createDataLayout());
    _module.setTargetTriple(target_triple);

    std::error_code ec;
    llvm::raw_fd_ostream dest(out_file, ec);
    // TODO: better way for error handling
    if (ec)
    {
        std::cerr << "Could not open file: " << ec.message() << std::endl;
        exit(-1);
    }

    // TODO: legacy is bad?
    llvm::legacy::PassManager pass;
    // TODO: better way for error handling
    // TODO: opportunity to select options
    if (target_machine->addPassesToEmitFile(pass, dest, nullptr, llvm::CGFT_ObjectFile))
    {
        std::cerr << "TargetMachine can't emit a file of this type!" << std::endl;
        exit(-1);
    }

    pass.run(_module);
    dest.flush();
}