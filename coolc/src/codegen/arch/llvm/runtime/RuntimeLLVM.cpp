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

#define FULL_METHOD_NAME(klass_id, method)                                                                             \
    Names::method_full_name(BaseClassesNames[klass_id], method, KlassLLVM::FULL_METHOD_DELIM)

RuntimeLLVM::RuntimeLLVM(llvm::Module &module)
    : _int32_type(llvm::Type::getInt32Ty(module.getContext())),
      _int64_type(llvm::Type::getInt64Ty(module.getContext())), _void_type(llvm::Type::getVoidTy(module.getContext())),
      _int8_type(llvm::Type::getInt8Ty(module.getContext())),
      _equals(module, SYMBOLS[RuntimeLLVMSymbols::EQUALS], _int32_type,
              {_void_type->getPointerTo(), _void_type->getPointerTo()}, *this),
      _object_abort(module, SYMBOLS[RuntimeLLVMSymbols::OBJECT_ABORT], _void_type->getPointerTo(),
                    {_void_type->getPointerTo()}, *this),
      _object_type_name(module, SYMBOLS[RuntimeLLVMSymbols::OBJECT_TYPE_NAME], _void_type->getPointerTo(),
                        {_void_type->getPointerTo()}, *this),
      _object_copy(module, SYMBOLS[RuntimeLLVMSymbols::OBJECT_COPY], _void_type->getPointerTo(),
                   {_void_type->getPointerTo()}, *this),
      _gc_alloc(module, SYMBOLS[RuntimeLLVMSymbols::GC_ALLOC], _void_type->getPointerTo(),
                {_int32_type, _int64_type, _void_type->getPointerTo()}, *this),
      _string_length(module, SYMBOLS[RuntimeLLVMSymbols::STRING_LENGTH], _void_type->getPointerTo(),
                     {_void_type->getPointerTo()}, *this),
      _string_concat(module, SYMBOLS[RuntimeLLVMSymbols::STRING_CONCAT], _void_type->getPointerTo(),
                     {_void_type->getPointerTo(), _void_type->getPointerTo()}, *this),
      _string_substr(module, SYMBOLS[RuntimeLLVMSymbols::STRING_SUBSTR], _void_type->getPointerTo(),
                     {_void_type->getPointerTo(), _void_type->getPointerTo(), _void_type->getPointerTo()}, *this),
      _io_out_string(module, SYMBOLS[RuntimeLLVMSymbols::IO_OUT_STRING], _void_type->getPointerTo(),
                     {_void_type->getPointerTo(), _void_type->getPointerTo()}, *this),
      _io_out_int(module, SYMBOLS[RuntimeLLVMSymbols::IO_OUT_INT], _void_type->getPointerTo(),
                  {_void_type->getPointerTo(), _void_type->getPointerTo()}, *this),
      _io_in_string(module, SYMBOLS[RuntimeLLVMSymbols::IO_IN_STRING], _void_type->getPointerTo(),
                    {_void_type->getPointerTo()}, *this),
      _io_in_int(module, SYMBOLS[RuntimeLLVMSymbols::IO_IN_INT], _void_type->getPointerTo(),
                 {_void_type->getPointerTo()}, *this)
{
    _header_layout_types[HeaderLayout::Mark] = llvm::IntegerType::get(module.getContext(), HeaderLayoutSizes::MarkSize);
    _header_layout_types[HeaderLayout::Tag] = llvm::IntegerType::get(module.getContext(), HeaderLayoutSizes::TagSize);
    _header_layout_types[HeaderLayout::Size] = llvm::IntegerType::get(module.getContext(), HeaderLayoutSizes::SizeSize);
    _header_layout_types[HeaderLayout::DispatchTable] = _void_type->getPointerTo();
}

const std::string RuntimeLLVM::SYMBOLS[RuntimeLLVMSymbolsSize] = {
    FULL_METHOD_NAME(BaseClasses::OBJECT, ObjectMethodsNames[ObjectMethods::ABORT]),
    FULL_METHOD_NAME(BaseClasses::OBJECT, ObjectMethodsNames[ObjectMethods::TYPE_NAME]),
    FULL_METHOD_NAME(BaseClasses::OBJECT, ObjectMethodsNames[ObjectMethods::COPY]),

    FULL_METHOD_NAME(BaseClasses::STRING, StringMethodsNames[StringMethods::LENGTH]),
    FULL_METHOD_NAME(BaseClasses::STRING, StringMethodsNames[StringMethods::CONCAT]),
    FULL_METHOD_NAME(BaseClasses::STRING, StringMethodsNames[StringMethods::SUBSTR]),

    FULL_METHOD_NAME(BaseClasses::IO, IOMethodsNames[IOMethods::OUT_STRING]),
    FULL_METHOD_NAME(BaseClasses::IO, IOMethodsNames[IOMethods::OUT_INT]),
    FULL_METHOD_NAME(BaseClasses::IO, IOMethodsNames[IOMethods::IN_STRING]),
    FULL_METHOD_NAME(BaseClasses::IO, IOMethodsNames[IOMethods::IN_INT]),

    "equals",
    "gc_alloc",
    "ClassNameTab",
    "ClassObjTab"};