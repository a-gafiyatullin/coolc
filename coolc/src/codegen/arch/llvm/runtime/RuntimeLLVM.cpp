#include "RuntimeLLVM.h"
#include "codegen/arch/llvm/klass/KlassLLVM.h"

using namespace codegen;

RuntimeMethod::RuntimeMethod(llvm::Module &module, const std::string &name, llvm::Type *ret,
                             const std::initializer_list<llvm::Type *> &args, RuntimeLLVM &runtime)
    : _func(llvm::Function::Create(llvm::FunctionType::get(ret, args, false), llvm::Function::ExternalLinkage, name,
                                   &module))
{
    runtime._symbol_by_name.insert({name, this});
}

RuntimeLLVM::RuntimeLLVM(llvm::Module &module)
    : _int32_type(llvm::Type::getInt32Ty(module.getContext())),
      _int64_type(llvm::Type::getInt64Ty(module.getContext())), _void_type(llvm::Type::getVoidTy(module.getContext())),
      _int8_type(llvm::Type::getInt8Ty(module.getContext())), _default_int(_int64_type),
      _stack_slot_type(_int8_type->getPointerTo()), _void_ptr_type(_void_type->getPointerTo()),
      _equals(module, SYMBOLS[RuntimeLLVMSymbols::EQUALS], _int32_type,
              {_void_type->getPointerTo(), _void_type->getPointerTo()}, *this),
      _gc_alloc(module, SYMBOLS[RuntimeLLVMSymbols::GC_ALLOC], _void_type->getPointerTo(),
                {_int32_type, _int64_type, _void_type->getPointerTo()}, *this),
      _case_abort(module, SYMBOLS[RuntimeLLVMSymbols::CASE_ABORT], _void_type, {_int32_type}, *this),
      _dispatch_abort(module, SYMBOLS[RuntimeLLVMSymbols::DISPATCH_ABORT], _void_type,
                      {_void_type->getPointerTo(), _int32_type}, *this),
      _case_abort_2(module, SYMBOLS[RuntimeLLVMSymbols::CASE_ABORT_2], _void_type,
                    {_void_type->getPointerTo(), _int32_type}, *this),
      _init_runtime(module, SYMBOLS[RuntimeLLVMSymbols::INIT_RUNTIME], _void_type,
                    {_int32_type, _int8_type->getPointerTo()->getPointerTo()}, *this),
      _finish_runtime(module, SYMBOLS[RuntimeLLVMSymbols::FINISH_RUNTIME], _void_type, {}, *this)
{
    _header_layout_types[HeaderLayout::Mark] =
        llvm::IntegerType::get(module.getContext(), HeaderLayoutSizes::MarkSize * BITS_PER_BYTE);
    _header_layout_types[HeaderLayout::Tag] =
        llvm::IntegerType::get(module.getContext(), HeaderLayoutSizes::TagSize * BITS_PER_BYTE);
    _header_layout_types[HeaderLayout::Size] =
        llvm::IntegerType::get(module.getContext(), HeaderLayoutSizes::SizeSize * BITS_PER_BYTE);
    _header_layout_types[HeaderLayout::DispatchTable] = _void_type->getPointerTo();
}

const std::string RuntimeLLVM::SYMBOLS[RuntimeLLVMSymbolsSize] = {
    "_equals",         "_case_abort",   "_case_abort_2", "_gc_alloc", "_dispatch_abort", "_init_runtime",
    "_finish_runtime", "class_nameTab", "class_objTab",  "_int_tag",  "_bool_tag",       "_string_tag"};

const std::string RuntimeLLVM::GC_STRATEGIES[RuntimeLLVMGCStrategySize] = {"shadow-stack"};