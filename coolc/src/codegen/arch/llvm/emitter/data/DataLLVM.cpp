#include "DataLLVM.h"
#include "codegen/emitter/data/Data.inline.h"

using namespace codegen;

#define FULL_METHOD_NAME(klass_id, method)                                                                             \
    Names::method_full_name(BaseClassesNames[klass_id], method, KlassLLVM::FULL_METHOD_DELIM)

DataLLVM::DataLLVM(const std::shared_ptr<KlassBuilder> &builder, llvm::Module &module, const RuntimeLLVM &runtime)
    : Data(builder), _module(module), _runtime(runtime)
{
    make_base_class(
        _builder->klass(BaseClassesNames[BaseClasses::OBJECT]), {},
        {runtime.method(FULL_METHOD_NAME(BaseClasses::OBJECT, ObjectMethodsNames[ObjectMethods::ABORT]))->_func,
         runtime.method(FULL_METHOD_NAME(BaseClasses::OBJECT, ObjectMethodsNames[ObjectMethods::TYPE_NAME]))->_func,
         runtime.method(FULL_METHOD_NAME(BaseClasses::OBJECT, ObjectMethodsNames[ObjectMethods::COPY]))->_func});

    make_base_class(
        _builder->klass(BaseClassesNames[BaseClasses::INT]), {_runtime.int64_type()},
        {runtime.method(FULL_METHOD_NAME(BaseClasses::OBJECT, ObjectMethodsNames[ObjectMethods::ABORT]))->_func,
         runtime.method(FULL_METHOD_NAME(BaseClasses::OBJECT, ObjectMethodsNames[ObjectMethods::TYPE_NAME]))->_func,
         runtime.method(FULL_METHOD_NAME(BaseClasses::OBJECT, ObjectMethodsNames[ObjectMethods::COPY]))->_func});

    // use 64 bit field for allignment
    make_base_class(
        _builder->klass(BaseClassesNames[BaseClasses::BOOL]), {_runtime.int64_type()},
        {runtime.method(FULL_METHOD_NAME(BaseClasses::OBJECT, ObjectMethodsNames[ObjectMethods::ABORT]))->_func,
         runtime.method(FULL_METHOD_NAME(BaseClasses::OBJECT, ObjectMethodsNames[ObjectMethods::TYPE_NAME]))->_func,
         runtime.method(FULL_METHOD_NAME(BaseClasses::OBJECT, ObjectMethodsNames[ObjectMethods::COPY]))->_func});

    make_base_class(
        _builder->klass(BaseClassesNames[BaseClasses::STRING]),
        {_classes[BaseClassesNames[BaseClasses::INT]]->getPointerTo(), _runtime.int8_type()->getPointerTo()},
        {runtime.method(FULL_METHOD_NAME(BaseClasses::OBJECT, ObjectMethodsNames[ObjectMethods::ABORT]))->_func,
         runtime.method(FULL_METHOD_NAME(BaseClasses::OBJECT, ObjectMethodsNames[ObjectMethods::TYPE_NAME]))->_func,
         runtime.method(FULL_METHOD_NAME(BaseClasses::OBJECT, ObjectMethodsNames[ObjectMethods::COPY]))->_func,
         runtime.method(FULL_METHOD_NAME(BaseClasses::STRING, StringMethodsNames[StringMethods::LENGTH]))->_func,
         runtime.method(FULL_METHOD_NAME(BaseClasses::STRING, StringMethodsNames[StringMethods::CONCAT]))->_func,
         runtime.method(FULL_METHOD_NAME(BaseClasses::STRING, StringMethodsNames[StringMethods::SUBSTR]))->_func});

    make_base_class(
        _builder->klass(BaseClassesNames[BaseClasses::IO]), {},
        {runtime.method(FULL_METHOD_NAME(BaseClasses::OBJECT, ObjectMethodsNames[ObjectMethods::ABORT]))->_func,
         runtime.method(FULL_METHOD_NAME(BaseClasses::OBJECT, ObjectMethodsNames[ObjectMethods::TYPE_NAME]))->_func,
         runtime.method(FULL_METHOD_NAME(BaseClasses::OBJECT, ObjectMethodsNames[ObjectMethods::COPY]))->_func,
         runtime.method(FULL_METHOD_NAME(BaseClasses::IO, IOMethodsNames[IOMethods::OUT_STRING]))->_func,
         runtime.method(FULL_METHOD_NAME(BaseClasses::IO, IOMethodsNames[IOMethods::OUT_INT]))->_func,
         runtime.method(FULL_METHOD_NAME(BaseClasses::IO, IOMethodsNames[IOMethods::IN_STRING]))->_func,
         runtime.method(FULL_METHOD_NAME(BaseClasses::IO, IOMethodsNames[IOMethods::IN_INT]))->_func});
}

llvm::GlobalVariable *DataLLVM::make_disp_table(const std::string &name, llvm::StructType *type,
                                                const std::vector<llvm::Constant *> &methods)
{
    _module.getOrInsertGlobal(name, type);

    auto *const disp_table_global = _module.getNamedGlobal(name);
    disp_table_global->setInitializer(llvm::ConstantStruct::get(type, methods));
    disp_table_global->setLinkage(llvm::GlobalValue::PrivateLinkage);
    disp_table_global->setConstant(true);

    // don't need allignment because all fields are pointers
    return disp_table_global;
}

void DataLLVM::make_init_method(const std::shared_ptr<Klass> &klass)
{
    const auto init_method_name = Names::init_method(klass->name());
    CODEGEN_VERBOSE_ONLY(LOG("Declare init method \"" + init_method_name + "\""));

    auto *const init_method = llvm::Function::Create(
        llvm::FunctionType::get(_runtime.void_type(), {class_struct(klass)->getPointerTo()}, false),
        llvm::Function::PrivateLinkage, init_method_name, &_module);

    // set receiver name
    init_method->getArg(0)->setName(SelfObject);
}

void DataLLVM::make_base_class(const std::shared_ptr<Klass> &klass, const std::vector<llvm::Type *> &additional_fields,
                               const std::vector<llvm::Constant *> &methods)
{
    const auto &class_name = klass->name();

    // dispatch table type entries
    std::vector<llvm::Type *> method_types;
    for (const auto &method : methods)
    {
        method_types.push_back(static_cast<const llvm::Function *>(method)->getType());
    }

    // header
    std::vector<llvm::Type *> fields;
    make_header(klass, fields);

    const auto &disp_tab_name = Names::disp_table(class_name);

    // set dispatch table
    auto *const dispatch_table_type =
        llvm::StructType::create(_module.getContext(), method_types, Names::type(disp_tab_name));
    fields.push_back(dispatch_table_type->getPointerTo());

    // add fields
    fields.insert(fields.end(), additional_fields.begin(), additional_fields.end());

    // record class structure and its dispatch table
    _classes.insert({class_name, llvm::StructType::create(_module.getContext(), fields, Names::prototype(class_name))});

    // create global variable for dispatch table
    _dispatch_tables.insert({class_name, make_disp_table(disp_tab_name, dispatch_table_type, methods)});

    make_init_method(klass);
}

void DataLLVM::make_header(const std::shared_ptr<Klass> &klass, std::vector<llvm::Type *> &fields)
{
    fields.push_back(_runtime.header_elem_type(HeaderLayout::Mark));
    fields.push_back(_runtime.header_elem_type(HeaderLayout::Tag));
    fields.push_back(_runtime.header_elem_type(HeaderLayout::Size));
}

void DataLLVM::class_struct_inner(const std::shared_ptr<Klass> &klass)
{
    const auto &class_name = klass->name();

    auto *const class_structure = llvm::StructType::create(_module.getContext(), Names::prototype(class_name));
    // for now we can reference to this structure. Need for recursion
    _classes.insert({class_name, class_structure});

    std::vector<llvm::Type *> fields;
    // add header
    make_header(klass, fields);
    fields.push_back(class_disp_tab(klass)->getType()); // dispatch table

    // add fields
    std::for_each(klass->fields_begin(), klass->fields_end(), [&fields, klass, this](const auto &field) {
        fields.push_back(class_struct(_builder->klass(field->_type->_string)));
    });

    class_structure->setBody(fields);

    make_init_method(klass);
}

void DataLLVM::class_disp_tab_inner(const std::shared_ptr<Klass> &klass)
{
    std::vector<llvm::Type *> method_types;
    std::vector<llvm::Constant *> methods;

    for_each(klass->methods_begin(), klass->methods_end(), [this, &methods, &method_types, &klass](const auto &method) {
        const auto method_full_name = klass->method_full_name(method.second->_object->_object);

        CODEGEN_VERBOSE_ONLY(LOG_ENTER("DECLARE METHOD \"" + method_full_name + "\""));

        // maybe already have such a method (from parent)? If so, just get a pointer to it, otherwise create a new one
        auto *func = _module.getFunction(method_full_name);
        if (!func)
        {
            std::vector<llvm::Type *> args;
            args.push_back(class_struct(_builder->klass(method.first->_string))->getPointerTo()); // this

            // formals
            const auto &method_formals = std::get<ast::MethodFeature>(method.second->_base);
            for (const auto &formal : method_formals._formals)
            {
                const auto &formal_type = formal->_type->_string;
                CODEGEN_VERBOSE_ONLY(LOG("Formal of type \"" + formal_type + "\""));
                args.push_back(class_struct(_builder->klass(formal_type))->getPointerTo());
            }

            const auto &return_type_name = method.second->_type->_string;
            CODEGEN_VERBOSE_ONLY(LOG("Return type: \"" + return_type_name + "\""));

            func = llvm::Function::Create(
                llvm::FunctionType::get(class_struct(_builder->klass(return_type_name))->getPointerTo(), args, false),
                llvm::Function::PrivateLinkage, method_full_name, &_module);

            // set names for args
            func->arg_begin()->setName(SelfObject);
            for (auto *arg = func->arg_begin() + 1; arg != func->arg_end(); arg++)
            {
                arg->setName(method_formals._formals[arg - func->arg_begin() - 1]->_type->_string);
            }
        }

        CODEGEN_VERBOSE_ONLY(LOG_EXIT("DECLARE METHOD \"" + method_full_name + "\""));

        method_types.push_back(func->getType());
        methods.push_back(func);
    });

    const auto &class_name = klass->name();
    const auto &disp_tab_name = Names::disp_table(class_name);
    _dispatch_tables.insert({class_name, make_disp_table(disp_tab_name,
                                                         llvm::StructType::create(_module.getContext(), method_types,
                                                                                  Names::type(disp_tab_name)),
                                                         methods)});
}

void DataLLVM::int_const_inner(const int64_t &value)
{
    const auto &klass = _builder->klass(BaseClassesNames[BaseClasses::INT]);
    auto *const int_struct = _classes.at(BaseClassesNames[BaseClasses::INT]);

    const auto const_name = Names::int_constant();

    auto *const const_global = static_cast<llvm::GlobalVariable *>(_module.getOrInsertGlobal(const_name, int_struct));
    const_global->setInitializer(llvm::ConstantStruct::get(
        int_struct, {llvm::ConstantInt::get(_runtime.header_elem_type(HeaderLayout::Mark), MarkWordDefaultValue, true),
                     llvm::ConstantInt::get(_runtime.header_elem_type(HeaderLayout::Tag), klass->tag(), true),
                     llvm::ConstantInt::get(_runtime.header_elem_type(HeaderLayout::Size), klass->size()),
                     _dispatch_tables.at(BaseClassesNames[BaseClasses::INT]),
                     llvm::ConstantInt::get(int_struct->getElementType(HeaderLayout::DispatchTable + 1), value,
                                            true)})); // value field

    const_global->setConstant(true);
    const_global->setLinkage(llvm::GlobalValue::PrivateLinkage);

    // don't need allignment because all fields are word-sized
    _int_constants.insert({value, const_global});
}

llvm::Constant *DataLLVM::make_char_string(const std::string &str)
{
    auto *const char_type = _runtime.int8_type();

    // 1. Initialize chars vector
    std::vector<llvm::Constant *> chars(str.length());
    for (auto i = 0; i < str.size(); i++)
    {
        chars[i] = llvm::ConstantInt::get(char_type, str[i]);
    }

    // 1b. add a zero terminator too
    chars.push_back(llvm::ConstantInt::get(char_type, 0));

    // 2. Initialize the string from the characters
    auto *const initializer = llvm::ConstantArray::get(llvm::ArrayType::get(char_type, chars.size()), chars);

    // 3. Create the declaration statement
    auto *const const_global =
        (llvm::GlobalVariable *)_module.getOrInsertGlobal(Names::char_str(str), initializer->getType());
    const_global->setInitializer(initializer);
    const_global->setConstant(true);
    const_global->setLinkage(llvm::GlobalValue::PrivateLinkage);

    // TODO: need this?
    // const_global->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Global);

    // 4. Return a cast to an i8*
    // TODO: does it work?
    return llvm::ConstantExpr::getBitCast(const_global, char_type->getPointerTo());
}

void DataLLVM::string_const_inner(const std::string &str)
{
    const auto &klass = _builder->klass(BaseClassesNames[BaseClasses::STRING]);
    auto *const str_struct = _classes.at(BaseClassesNames[BaseClasses::STRING]);

    const auto const_name = Names::string_constant();

    auto *const const_global = static_cast<llvm::GlobalVariable *>(_module.getOrInsertGlobal(const_name, str_struct));
    const_global->setInitializer(llvm::ConstantStruct::get(
        str_struct,
        {llvm::ConstantInt::get(_runtime.header_elem_type(HeaderLayout::Mark), MarkWordDefaultValue, true),
         llvm::ConstantInt::get(_runtime.header_elem_type(HeaderLayout::Tag), klass->tag(), true),
         llvm::ConstantInt::get(_runtime.header_elem_type(HeaderLayout::Size), klass->size()),
         _dispatch_tables.at(BaseClassesNames[BaseClasses::STRING]), int_const(str.length()), // length field
         make_char_string(str)}));                                                            // string field

    const_global->setConstant(true);
    const_global->setLinkage(llvm::GlobalValue::PrivateLinkage);

    // don't need allignment because all fields are word-sized
    _string_constants.insert({str, const_global});
}

void DataLLVM::bool_const_inner(const bool &value)
{
    const auto &klass = _builder->klass(BaseClassesNames[BaseClasses::BOOL]);
    auto *const bool_struct = _classes.at(BaseClassesNames[BaseClasses::BOOL]);

    const auto const_name = Names::bool_constant();

    auto *const const_global = static_cast<llvm::GlobalVariable *>(_module.getOrInsertGlobal(const_name, bool_struct));
    const_global->setInitializer(llvm::ConstantStruct::get(
        bool_struct, {llvm::ConstantInt::get(_runtime.header_elem_type(HeaderLayout::Mark), MarkWordDefaultValue, true),
                      llvm::ConstantInt::get(_runtime.header_elem_type(HeaderLayout::Tag), klass->tag(), true),
                      llvm::ConstantInt::get(_runtime.header_elem_type(HeaderLayout::Size), klass->size()),
                      _dispatch_tables.at(BaseClassesNames[BaseClasses::BOOL]),
                      llvm::ConstantInt::get(bool_struct->getElementType(HeaderLayout::DispatchTable + 1), value,
                                             true)})); // value field

    const_global->setConstant(true);
    const_global->setLinkage(llvm::GlobalValue::PrivateLinkage);

    // don't need allignment because all fields are word-sized
    _bool_constants.insert({value, const_global});
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