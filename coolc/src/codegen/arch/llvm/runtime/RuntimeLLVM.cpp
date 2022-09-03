#include "RuntimeLLVM.h"
#include "codegen/arch/llvm/klass/KlassLLVM.h"

using namespace codegen;

RuntimeMethod::RuntimeMethod(llvm::Module &module, const std::string &name, llvm::Type *ret,
                             const std::initializer_list<llvm::Type *> &args, bool need_gc, RuntimeLLVM &runtime)
    : _func(llvm::Function::Create(llvm::FunctionType::get(ret, args, false), llvm::Function::ExternalLinkage, name,
                                   &module))
{
    runtime._symbol_by_name.insert({name, this});

    // set gc
    const auto gc_name = runtime.gc_strategy_name();
    if (need_gc && gc_name != runtime.GC_DEFAULT_NAME)
    {
        _func->setGC(gc_name);
    }
}

RuntimeLLVM::RuntimeLLVM(llvm::Module &module)
    : _int32_type(llvm::Type::getInt32Ty(module.getContext())),
      _int64_type(llvm::Type::getInt64Ty(module.getContext())), _void_type(llvm::Type::getVoidTy(module.getContext())),
      _int8_type(llvm::Type::getInt8Ty(module.getContext())), _default_int(_int64_type),
      _stack_slot_type(_int8_type->getPointerTo(HEAP_ADDR_SPACE)),
      _heap_ptr_type(_void_type->getPointerTo(HEAP_ADDR_SPACE)),

      _equals(module, SYMBOLS[RuntimeLLVMSymbols::EQUALS], _int32_type,
              {_void_type->getPointerTo(HEAP_ADDR_SPACE), _void_type->getPointerTo(HEAP_ADDR_SPACE)}, true, *this),
      _gc_alloc(module, SYMBOLS[RuntimeLLVMSymbols::GC_ALLOC], _void_type->getPointerTo(HEAP_ADDR_SPACE),
                {_int32_type, _int64_type, _void_type->getPointerTo()}, true, *this),
      _case_abort(module, SYMBOLS[RuntimeLLVMSymbols::CASE_ABORT], _void_type, {_int32_type}, false, *this),
      _dispatch_abort(module, SYMBOLS[RuntimeLLVMSymbols::DISPATCH_ABORT], _void_type,
                      {_void_type->getPointerTo(HEAP_ADDR_SPACE), _int32_type}, true, *this),
      _case_abort_2(module, SYMBOLS[RuntimeLLVMSymbols::CASE_ABORT_2], _void_type,
                    {_void_type->getPointerTo(HEAP_ADDR_SPACE), _int32_type}, true, *this),
      _init_runtime(module, SYMBOLS[RuntimeLLVMSymbols::INIT_RUNTIME], _void_type,
                    {_int32_type, _int8_type->getPointerTo()->getPointerTo()}, false, *this),
      _finish_runtime(module, SYMBOLS[RuntimeLLVMSymbols::FINISH_RUNTIME], _void_type, {}, false, *this)
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