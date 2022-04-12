#include "DataLLVM.h"
#include "codegen/emitter/data/Data.inline.h"

using namespace codegen;

#define FULL_METHOD_NAME(klass_id, method)                                                                             \
    NameConstructor::method_full_name(BaseClassesNames[klass_id], method, KlassLLVM::FULL_METHOD_DELIM)

DataLLVM::DataLLVM(const std::shared_ptr<KlassBuilder> &builder, llvm::Module &module, const RuntimeLLVM &runtime)
    : Data(builder), _module(module), _runtime(runtime)
{
    construct_base_class(
        _builder->klass(BaseClassesNames[BaseClasses::OBJECT]), {},
        {runtime.method(FULL_METHOD_NAME(BaseClasses::OBJECT, ObjectMethodsNames[ObjectMethods::ABORT]))->_func,
         runtime.method(FULL_METHOD_NAME(BaseClasses::OBJECT, ObjectMethodsNames[ObjectMethods::TYPE_NAME]))->_func,
         runtime.method(FULL_METHOD_NAME(BaseClasses::OBJECT, ObjectMethodsNames[ObjectMethods::COPY]))->_func});

    // TODO: check this twice for field with value
    construct_base_class(
        _builder->klass(BaseClassesNames[BaseClasses::INT]), {llvm::Type::getInt64Ty(_module.getContext())},
        {runtime.method(FULL_METHOD_NAME(BaseClasses::OBJECT, ObjectMethodsNames[ObjectMethods::ABORT]))->_func,
         runtime.method(FULL_METHOD_NAME(BaseClasses::OBJECT, ObjectMethodsNames[ObjectMethods::TYPE_NAME]))->_func,
         runtime.method(FULL_METHOD_NAME(BaseClasses::OBJECT, ObjectMethodsNames[ObjectMethods::COPY]))->_func});

    // TODO: check this twice for field with value. Too big?
    construct_base_class(
        _builder->klass(BaseClassesNames[BaseClasses::BOOL]), {llvm::Type::getInt64Ty(_module.getContext())},
        {runtime.method(FULL_METHOD_NAME(BaseClasses::OBJECT, ObjectMethodsNames[ObjectMethods::ABORT]))->_func,
         runtime.method(FULL_METHOD_NAME(BaseClasses::OBJECT, ObjectMethodsNames[ObjectMethods::TYPE_NAME]))->_func,
         runtime.method(FULL_METHOD_NAME(BaseClasses::OBJECT, ObjectMethodsNames[ObjectMethods::COPY]))->_func});

    // TODO: check this twice
    // TODO: int8 ptr for char*?
    // TODO: alignment?
    construct_base_class(
        _builder->klass(BaseClassesNames[BaseClasses::STRING]),
        {_classes[semant::Semant::int_type()->_string], llvm::Type::getInt8PtrTy(_module.getContext())},
        {runtime.method(FULL_METHOD_NAME(BaseClasses::OBJECT, ObjectMethodsNames[ObjectMethods::ABORT]))->_func,
         runtime.method(FULL_METHOD_NAME(BaseClasses::OBJECT, ObjectMethodsNames[ObjectMethods::TYPE_NAME]))->_func,
         runtime.method(FULL_METHOD_NAME(BaseClasses::OBJECT, ObjectMethodsNames[ObjectMethods::COPY]))->_func,
         runtime.method(FULL_METHOD_NAME(BaseClasses::STRING, StringMethodsNames[StringMethods::LENGTH]))->_func,
         runtime.method(FULL_METHOD_NAME(BaseClasses::STRING, StringMethodsNames[StringMethods::CONCAT]))->_func,
         runtime.method(FULL_METHOD_NAME(BaseClasses::STRING, StringMethodsNames[StringMethods::SUBSTR]))->_func});

    construct_base_class(
        _builder->klass(BaseClassesNames[BaseClasses::IO]), {},
        {runtime.method(FULL_METHOD_NAME(BaseClasses::OBJECT, ObjectMethodsNames[ObjectMethods::ABORT]))->_func,
         runtime.method(FULL_METHOD_NAME(BaseClasses::OBJECT, ObjectMethodsNames[ObjectMethods::TYPE_NAME]))->_func,
         runtime.method(FULL_METHOD_NAME(BaseClasses::OBJECT, ObjectMethodsNames[ObjectMethods::COPY]))->_func,
         runtime.method(FULL_METHOD_NAME(BaseClasses::IO, IOMethodsNames[IOMethods::OUT_STRING]))->_func,
         runtime.method(FULL_METHOD_NAME(BaseClasses::IO, IOMethodsNames[IOMethods::OUT_INT]))->_func,
         runtime.method(FULL_METHOD_NAME(BaseClasses::IO, IOMethodsNames[IOMethods::IN_STRING]))->_func,
         runtime.method(FULL_METHOD_NAME(BaseClasses::IO, IOMethodsNames[IOMethods::IN_INT]))->_func});
}

void DataLLVM::construct_base_class(const std::shared_ptr<Klass> &klass,
                                    const std::vector<llvm::Type *> &additional_fields,
                                    const std::vector<llvm::Constant *> &methods)
{
    const std::string &class_name = klass->name();
    // dispatch table type entries
    std::vector<llvm::Type *> method_types;
    for (const auto &method : methods)
    {
        method_types.push_back(static_cast<const llvm::Function *>(method)->getFunctionType());
    }

    // header
    std::vector<llvm::Type *> fields;
    construct_header(klass, fields);

    // set dispatch table
    auto *const dispatch_table_type =
        llvm::StructType::create(_module.getContext(), method_types,
                                 NameConstructor::disp_table(class_name) + static_cast<std::string>(TYPE_SUFFIX));
    fields.push_back(dispatch_table_type);

    // add fields
    fields.insert(fields.end(), additional_fields.begin(), additional_fields.end());

    // record class structure and its dispatch table
    _classes.insert(
        {class_name, llvm::StructType::create(_module.getContext(), fields, NameConstructor::prototype(class_name))});
    _dispatch_tables.insert({class_name, llvm::ConstantStruct::get(dispatch_table_type, methods)});
}

void DataLLVM::construct_header(const std::shared_ptr<Klass> &klass, std::vector<llvm::Type *> &fields)
{
    // TODO: maybe too big types for header?
    fields.push_back(llvm::Type::getInt64Ty(_module.getContext())); // mark
    fields.push_back(llvm::Type::getInt64Ty(_module.getContext())); // tag
    fields.push_back(llvm::Type::getInt64Ty(_module.getContext())); // size
}

void DataLLVM::class_struct_inner(const std::shared_ptr<Klass> &klass)
{
    const auto &class_name = klass->name();

    auto *const class_structure =
        llvm::StructType::create(_module.getContext(), NameConstructor::prototype(class_name));
    // for now we can reference to this structure. Need for recursion
    _classes.insert({class_name, class_structure});

    std::vector<llvm::Type *> fields;
    // add header
    construct_header(klass, fields);
    fields.push_back(class_disp_tab(klass)->getType()); // dispatch table

    // add fields
    std::for_each(klass->fields_begin(), klass->fields_end(), [&fields, klass, this](const auto &field) {
        fields.push_back(class_struct(_builder->klass(field->_type->_string)));
    });

    class_structure->setBody(fields);
}

void DataLLVM::class_disp_tab_inner(const std::shared_ptr<Klass> &klass)
{
    const auto &class_name = klass->name();

    const std::string init_method_name = NameConstructor::init_method(class_name);
    CODEGEN_VERBOSE_ONLY(LOG("Declare init method \"" + init_method_name + "\""));
    // create init method
    // TODO: CommonLinkage is good for such methods?
    llvm::Function::Create(llvm::FunctionType::get(llvm::Type::getVoidTy(_module.getContext()),
                                                   {_runtime._void_ptr_type, _runtime._void_ptr_type,
                                                    llvm::Type::getInt32Ty(_module.getContext())},
                                                   false),
                           llvm::Function::CommonLinkage, init_method_name, &_module);

    std::vector<llvm::Type *> method_types;
    std::vector<llvm::Constant *> methods;

    for_each(klass->methods_begin(), klass->methods_end(), [this, &methods, &method_types, &klass](const auto &method) {
        const std::string method_full_name = klass->method_full_name(method.second->_object->_object);

        CODEGEN_VERBOSE_ONLY(LOG_ENTER("declare method \"" + method_full_name + "\""));

        // maybe already have such a method (from parent)? If so, just get a pointer to it, otherwise create new one
        auto *func = _module.getFunction(method_full_name);
        if (!func)
        {
            std::vector<llvm::Type *> args;
            args.push_back(class_struct(_builder->klass(method.first->_string))->getPointerTo()); // this

            // formals
            const auto &method_formals = std::get<ast::MethodFeature>(method.second->_base);
            for (const auto &formal : method_formals._formals)
            {
                const std::string &formal_name = formal->_type->_string;
                CODEGEN_VERBOSE_ONLY(LOG("Formal with type: " + formal_name));
                args.push_back(class_struct(_builder->klass(formal_name))->getPointerTo());
            }

            const std::string &return_type_name = method.second->_type->_string;
            CODEGEN_VERBOSE_ONLY(LOG("Return type: " + return_type_name));
            // TODO: CommonLinkage is good for such methods?
            func = llvm::Function::Create(
                llvm::FunctionType::get(class_struct(_builder->klass(return_type_name))->getPointerTo(), args, false),
                llvm::Function::CommonLinkage, method_full_name, &_module);

            // set names for args
            func->arg_begin()->setName(SelfObject);
            for (auto *arg = func->arg_begin() + 1; arg != func->arg_end(); arg++)
            {
                arg->setName(method_formals._formals[arg - func->arg_begin() - 1]->_type->_string);
            }
        }

        CODEGEN_VERBOSE_ONLY(LOG_EXIT("declare method \"" + method_full_name + "\""));

        method_types.push_back(func->getFunctionType());
        methods.push_back(func);
    });

    auto *const dispatch_table_type =
        llvm::StructType::create(_module.getContext(), method_types,
                                 NameConstructor::disp_table(class_name) + static_cast<std::string>(TYPE_SUFFIX));
    _dispatch_tables.insert({class_name, llvm::ConstantStruct::get(dispatch_table_type, methods)});
}

void DataLLVM::int_const_inner(const int64_t &value)
{
    // TODO: are constants unique?
    // TODO: should provide a name?
    const std::string &class_name = semant::Semant::int_type()->_string;
    const auto &klass = _builder->klass(class_name);

    // TODO: do we realy need this "true" in getElementType?
    llvm::StructType *integer_struct = _classes.at(class_name);
    _int_constants.insert(
        {value, llvm::ConstantStruct::get(
                    integer_struct,
                    {llvm::ConstantInt::get(integer_struct->getElementType(HeaderLayout::Mark), 0, true),
                     llvm::ConstantInt::get(integer_struct->getElementType(HeaderLayout::Tag), klass->tag(), true),
                     llvm::ConstantInt::get(integer_struct->getElementType(HeaderLayout::Size), klass->size(), true),
                     _dispatch_tables.at(class_name),
                     llvm::ConstantInt::get(integer_struct->getElementType(HeaderLayout::DispatchTable + 1), value,
                                            true)})}); // value field
}

void DataLLVM::string_const_inner(const std::string &str)
{
    // TODO: dummy
    _string_constants.insert({str, nullptr});
}

void DataLLVM::bool_const_inner(const bool &value)
{
}

void DataLLVM::gen_class_obj_tab()
{
    for (const auto &klass : _builder->klasses())
    {
        class_struct(klass);
    }
}

void DataLLVM::gen_class_name_tab()
{
}

void DataLLVM::emit_inner(const std::string &out_file)
{
}