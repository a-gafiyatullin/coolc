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
      _int8_type(llvm::Type::getInt8Ty(module.getContext())),
      _equals(module, SYMBOLS[RuntimeLLVMSymbols::EQUALS], _int32_type,
              {_void_type->getPointerTo(), _void_type->getPointerTo()}, *this),
      _gc_alloc(module, SYMBOLS[RuntimeLLVMSymbols::GC_ALLOC], _void_type->getPointerTo(),
                {_int32_type, _int64_type, _void_type->getPointerTo()}, *this),
      _case_abort(module, SYMBOLS[RuntimeLLVMSymbols::CASE_ABORT], _void_type, {_int32_type}, *this)
{
    _header_layout_types[HeaderLayout::Mark] = llvm::IntegerType::get(module.getContext(), HeaderLayoutSizes::MarkSize);
    _header_layout_types[HeaderLayout::Tag] = llvm::IntegerType::get(module.getContext(), HeaderLayoutSizes::TagSize);
    _header_layout_types[HeaderLayout::Size] = llvm::IntegerType::get(module.getContext(), HeaderLayoutSizes::SizeSize);
    _header_layout_types[HeaderLayout::DispatchTable] = _void_type->getPointerTo();
}

const std::string RuntimeLLVM::SYMBOLS[RuntimeLLVMSymbolsSize] = {"equals", "case_abort", "gc_alloc", "ClassNameTab",
                                                                  "ClassObjTab"};