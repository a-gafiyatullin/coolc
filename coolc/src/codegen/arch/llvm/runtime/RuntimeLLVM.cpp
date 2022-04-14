#include "RuntimeLLVM.h"

using namespace codegen;

RuntimeMethod::RuntimeMethod(llvm::Module &module, const std::string &name, llvm::Type *ret,
                             const std::initializer_list<llvm::Type *> &args, RuntimeLLVM &runtime)
    : _func(llvm::Function::Create(llvm::FunctionType::get(ret, args, false), llvm::Function::ExternalLinkage, name,
                                   &module))
{
    runtime._method_by_name.insert({name, this});
}

#define FULL_METHOD_NAME(klass_id, method)                                                                             \
    NameConstructor::method_full_name(BaseClassesNames[klass_id], method, KlassLLVM::FULL_METHOD_DELIM)

// TODO: initialize runtime structures
RuntimeLLVM::RuntimeLLVM(llvm::Module &module)
    : Runtime(nullptr, nullptr), _void_ptr_type(llvm::Type::getVoidTy(module.getContext())->getPointerTo()),
      _int32_type(llvm::Type::getInt32Ty(module.getContext())),
      _int64_type(llvm::Type::getInt64Ty(module.getContext())), _void_type(llvm::Type::getVoidTy(module.getContext())),
      _equals(module, RuntimeMethodsNames[RuntimeMethods::EQUALS], _int32_type, {_void_ptr_type, _void_ptr_type},
              *this),
      _object_abort(module, FULL_METHOD_NAME(BaseClasses::OBJECT, ObjectMethodsNames[ObjectMethods::ABORT]),
                    _void_ptr_type, {_void_ptr_type}, *this),
      _object_type_name(module, FULL_METHOD_NAME(BaseClasses::OBJECT, ObjectMethodsNames[ObjectMethods::TYPE_NAME]),
                        _void_ptr_type, {_void_ptr_type}, *this),
      _object_copy(module, FULL_METHOD_NAME(BaseClasses::OBJECT, ObjectMethodsNames[ObjectMethods::COPY]),
                   _void_ptr_type, {_void_ptr_type}, *this),
      _gc_alloc(module, RuntimeMethodsNames[RuntimeMethods::GC_ALLOC], _void_ptr_type,
                {_int32_type, _int64_type, _void_ptr_type}, *this),
      _gc_alloc_by_tag(module, RuntimeMethodsNames[RuntimeMethods::GC_ALLOC_BY_TAG], _void_ptr_type,
                       {llvm::Type::getInt64Ty(module.getContext())}, *this),
      _string_length(module, FULL_METHOD_NAME(BaseClasses::STRING, StringMethodsNames[StringMethods::LENGTH]),
                     _void_ptr_type, {_void_ptr_type}, *this),
      _string_concat(module, FULL_METHOD_NAME(BaseClasses::STRING, StringMethodsNames[StringMethods::CONCAT]),
                     _void_ptr_type, {_void_ptr_type, _void_ptr_type}, *this),
      _string_substr(module, FULL_METHOD_NAME(BaseClasses::STRING, StringMethodsNames[StringMethods::SUBSTR]),
                     _void_ptr_type, {_void_ptr_type, _void_ptr_type, _void_ptr_type}, *this),
      _io_out_string(module, FULL_METHOD_NAME(BaseClasses::IO, IOMethodsNames[IOMethods::OUT_STRING]), _void_ptr_type,
                     {_void_ptr_type, _void_ptr_type}, *this),
      _io_out_int(module, FULL_METHOD_NAME(BaseClasses::IO, IOMethodsNames[IOMethods::OUT_INT]), _void_ptr_type,
                  {_void_ptr_type, _void_ptr_type}, *this),
      _io_in_string(module, FULL_METHOD_NAME(BaseClasses::IO, IOMethodsNames[IOMethods::IN_STRING]), _void_ptr_type,
                    {_void_ptr_type}, *this),
      _io_in_int(module, FULL_METHOD_NAME(BaseClasses::IO, IOMethodsNames[IOMethods::IN_INT]), _void_ptr_type,
                 {_void_ptr_type}, *this)
{
}