#include "RuntimeLLVM.h"

using namespace codegen;

RuntimeMethod::RuntimeMethod(llvm::Module &module, const std::string &name, llvm::Type *ret,
                             const std::initializer_list<llvm::Type *> &args, RuntimeLLVM &runtime)
    : _func(llvm::Function::Create(llvm::FunctionType::get(ret, args, false), llvm::Function::ExternalLinkage, name,
                                   &module))
{
    runtime._method_by_name.insert({name, this});
}

// TODO: initialize runtime structures
RuntimeLLVM::RuntimeLLVM(llvm::Module &module)
    : Runtime(nullptr, nullptr), _void_ptr_type(llvm::Type::getVoidTy(module.getContext())->getPointerTo()),
      _equals(module, RuntimeMethodsNames[RuntimeMethods::EQUALS], llvm::Type::getInt32Ty(module.getContext()),
              {_void_ptr_type, _void_ptr_type}, *this),
      _object_abort(module, ObjectMethodsNames[ObjectMethods::ABORT], _void_ptr_type, {_void_ptr_type}, *this),
      _object_type_name(module, ObjectMethodsNames[ObjectMethods::TYPE_NAME], _void_ptr_type, {_void_ptr_type}, *this),
      _object_copy(module, ObjectMethodsNames[ObjectMethods::COPY], _void_ptr_type, {_void_ptr_type}, *this),
      _gc_alloc(module, RuntimeMethodsNames[RuntimeMethods::GC_ALLOC], _void_ptr_type,
                {llvm::Type::getInt32Ty(module.getContext())}, *this),
      _gc_alloc_by_tag(module, RuntimeMethodsNames[RuntimeMethods::GC_ALLOC_BY_TAG], _void_ptr_type,
                       {llvm::Type::getInt64Ty(module.getContext())}, *this)
{
}