#include "DataLLVM.h"
#include "codegen/emitter/data/Data.inline.h"

using namespace codegen;

DataLLVM::DataLLVM(const std::shared_ptr<KlassBuilder> &builder, llvm::Module &module, const RuntimeLLVM &runtime)
    : Data(builder), _module(module), _runtime(runtime)
{
    // publish basic classes structures
    for (auto i = static_cast<int>(BaseClasses::OBJECT); i < BaseClasses::SELF_TYPE; i++)
    {
        _classes.insert(
            {BaseClassesNames[i],
             llvm::StructType::create(_module.getContext(), _builder->klass(BaseClassesNames[i])->prototype())});
    }

    make_base_class(_builder->klass(BaseClassesNames[BaseClasses::OBJECT]), {});

    make_base_class(_builder->klass(BaseClassesNames[BaseClasses::INT]), {_runtime.default_int()});

    // use 64 bit field for allignment
    make_base_class(_builder->klass(BaseClassesNames[BaseClasses::BOOL]), {_runtime.default_int()});

    make_base_class(
        _builder->klass(BaseClassesNames[BaseClasses::STRING]),
        {_classes[BaseClassesNames[BaseClasses::INT]]->getPointerTo(), _runtime.int8_type()->getPointerTo()});

    make_base_class(_builder->klass(BaseClassesNames[BaseClasses::IO]), {});

    // create constants for basic classes tags
    // order in tags synchronized with RuntimeLLVM::RuntimeLLVMSymbols
    const int tags[] = {_builder->tag(BaseClassesNames[BaseClasses::INT]),
                        _builder->tag(BaseClassesNames[BaseClasses::BOOL]),
                        _builder->tag(BaseClassesNames[BaseClasses::STRING])};
    auto *const tag_type = _runtime.header_elem_type(HeaderLayout::Tag);
    for (int i = RuntimeLLVM::RuntimeLLVMSymbols::INT_TAG_NAME; i <= RuntimeLLVM::RuntimeLLVMSymbols::STRING_TAG_NAME;
         i++)
    {
        make_constant(_runtime.symbol_name(i), tag_type,
                      llvm::ConstantInt::get(tag_type, tags[i - RuntimeLLVM::RuntimeLLVMSymbols::INT_TAG_NAME]));
    }
}

llvm::GlobalVariable *DataLLVM::make_constant_struct(const std::string &name, llvm::StructType *type,
                                                     const std::vector<llvm::Constant *> &elements)
{

    auto *const constant = static_cast<llvm::GlobalVariable *>(_module.getOrInsertGlobal(name, type));
    constant->setInitializer(llvm::ConstantStruct::get(type, elements));
    constant->setLinkage(llvm::GlobalValue::ExternalLinkage);
    constant->setConstant(true);

    // TODO: need allignment?
    return constant;
}

llvm::GlobalVariable *DataLLVM::make_constant(const std::string &name, llvm::Type *type, llvm::Constant *element)
{
    auto *const constant = static_cast<llvm::GlobalVariable *>(_module.getOrInsertGlobal(name, type));
    GUARANTEE_DEBUG(constant);

    constant->setInitializer(element);
    constant->setLinkage(llvm::GlobalValue::ExternalLinkage);
    constant->setConstant(true);

    // TODO: need allignment?
    return constant;
}

llvm::GlobalVariable *DataLLVM::make_constant_array(const std::string &name, llvm::ArrayType *type,
                                                    const std::vector<llvm::Constant *> &elements)
{
    auto *const constant = static_cast<llvm::GlobalVariable *>(_module.getOrInsertGlobal(name, type));
    GUARANTEE_DEBUG(constant);

    constant->setInitializer(llvm::ConstantArray::get(type, elements));
    constant->setLinkage(llvm::GlobalValue::ExternalLinkage);
    constant->setConstant(true);

    // TODO: need allignment?
    return constant;
}

void DataLLVM::make_init_method(const std::shared_ptr<Klass> &klass)
{
    const auto init_method_name = klass->init_method();
    CODEGEN_VERBOSE_ONLY(LOG("Declare init method \"" + init_method_name + "\""));

    auto *const init_method = llvm::Function::Create(
        llvm::FunctionType::get(_runtime.void_type(), {class_struct(klass)->getPointerTo()}, false),
        llvm::Function::ExternalLinkage, init_method_name, &_module);

    // set receiver name
    init_method->getArg(0)->setName(SelfObject);
}

void DataLLVM::make_base_class(const std::shared_ptr<Klass> &klass, const std::vector<llvm::Type *> &additional_fields)
{
    // header
    std::vector<llvm::Type *> fields;
    make_header(klass, fields);

    // set dispatch table
    auto *const dispatch_table = class_disp_tab(klass);
    fields.push_back(dispatch_table->getType());

    // add fields
    fields.insert(fields.end(), additional_fields.begin(), additional_fields.end());

    class_struct(klass)->setBody(fields);

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
    auto *const class_structure = llvm::StructType::create(_module.getContext(), klass->prototype());
    // for now we can reference to this structure. Need for recursion
    _classes.insert({klass->name(), class_structure});

    std::vector<llvm::Type *> fields;
    // add header
    make_header(klass, fields);
    fields.push_back(class_disp_tab(klass)->getType()); // dispatch table

    // add fields
    std::for_each(klass->fields_begin(), klass->fields_end(), [&fields, klass, this](const auto &field) {
        fields.push_back(
            class_struct(_builder->klass(semant::Semant::exact_type(field->_type, klass->klass())->_string))
                ->getPointerTo());
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

        const auto &return_type = method.second->_type;

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
                // TODO: can be SELF_TYPE here?
                const auto &formal_type = formal->_type->_string;
                CODEGEN_VERBOSE_ONLY(LOG("Formal of type \"" + formal_type + "\""));
                args.push_back(class_struct(_builder->klass(formal_type))->getPointerTo());
            }

            CODEGEN_VERBOSE_ONLY(LOG("Return type: \"" + return_type->_string + "\""));

            const auto return_klass_struct =
                class_struct(_builder->klass(semant::Semant::exact_type(return_type, klass->klass())->_string))
                    ->getPointerTo();

            // maybe we already created this method in recursive call of class_struct
            func = _module.getFunction(method_full_name);
            if (!func)
            {
                func = llvm::Function::Create(llvm::FunctionType::get(return_klass_struct, args, false),
                                              llvm::Function::ExternalLinkage, method_full_name, &_module);

                // set names for args
                func->arg_begin()->setName(SelfObject);
                for (auto *arg = func->arg_begin() + 1; arg != func->arg_end(); arg++)
                {
                    arg->setName(method_formals._formals[arg - func->arg_begin() - 1]->_object->_object);
                }
            }
        }

        CODEGEN_VERBOSE_ONLY(LOG_EXIT("DECLARE METHOD \"" + method_full_name + "\""));

        method_types.push_back(func->getType());
        methods.push_back(func);
    });

    const auto &disp_tab_name = klass->disp_tab();
    _dispatch_tables.insert(
        {klass->name(), make_constant_struct(disp_tab_name,
                                             llvm::StructType::create(_module.getContext(), method_types,
                                                                      Names::name(Names::Comment::TYPE, disp_tab_name)),
                                             methods)});
}

void DataLLVM::int_const_inner(const int64_t &value)
{
    const auto &klass_name = BaseClassesNames[BaseClasses::INT];

    const auto &klass = _builder->klass(klass_name);
    auto *const int_struct = _classes.at(klass_name);

    auto *const constant_int = make_constant_struct(
        Names::int_constant(), int_struct,
        {llvm::ConstantInt::get(_runtime.header_elem_type(HeaderLayout::Mark), MarkWordSetValue, true),
         llvm::ConstantInt::get(_runtime.header_elem_type(HeaderLayout::Tag), klass->tag(), true),
         llvm::ConstantInt::get(_runtime.header_elem_type(HeaderLayout::Size), klass->size()),
         _dispatch_tables.at(klass_name),
         llvm::ConstantInt::get(int_struct->getElementType(HeaderLayout::DispatchTable + 1), value,
                                true)}); // value field

    _int_constants.insert({value, constant_int});
}

llvm::Constant *DataLLVM::make_char_string(const std::string &str)
{
    auto *const char_type = _runtime.int8_type();

    std::vector<llvm::Constant *> chars;
    for (auto i = 0; i < str.size(); i++)
    {
        chars.push_back(llvm::ConstantInt::get(char_type, str[i]));
    }

    chars.push_back(llvm::ConstantInt::get(char_type, 0));

    auto *const const_char_str = make_constant_array(Names::name(Names::Comment::CHAR_STRING, str),
                                                     llvm::ArrayType::get(char_type, chars.size()), chars);

    // TODO: need this?
    // const_global->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Global);

    return const_char_str;
}

void DataLLVM::string_const_inner(const std::string &str)
{
    const auto &klass_name = BaseClassesNames[BaseClasses::STRING];
    const auto &klass = _builder->klass(klass_name);

    auto *const constant_str = make_constant_struct(
        Names::string_constant(), _classes.at(klass_name),
        {llvm::ConstantInt::get(_runtime.header_elem_type(HeaderLayout::Mark), MarkWordSetValue, true),
         llvm::ConstantInt::get(_runtime.header_elem_type(HeaderLayout::Tag), klass->tag(), true),
         llvm::ConstantInt::get(_runtime.header_elem_type(HeaderLayout::Size), klass->size()),
         _dispatch_tables.at(klass_name), int_const(str.length()), // length field
         make_char_string(str)});                                  // string field

    _string_constants.insert({str, constant_str});
}

void DataLLVM::bool_const_inner(const bool &value)
{
    const auto &klass_name = BaseClassesNames[BaseClasses::BOOL];

    const auto &klass = _builder->klass(klass_name);
    auto *const bool_struct = _classes.at(klass_name);

    auto *const constant_bool = make_constant_struct(
        Names::bool_constant(), bool_struct,
        {llvm::ConstantInt::get(_runtime.header_elem_type(HeaderLayout::Mark), MarkWordSetValue, true),
         llvm::ConstantInt::get(_runtime.header_elem_type(HeaderLayout::Tag), klass->tag(), true),
         llvm::ConstantInt::get(_runtime.header_elem_type(HeaderLayout::Size), klass->size()),
         _dispatch_tables.at(klass_name),
         llvm::ConstantInt::get(bool_struct->getElementType(HeaderLayout::DispatchTable + 1), value,
                                true)}); // value field

    _bool_constants.insert({value, constant_bool});
}

void DataLLVM::gen_class_obj_tab()
{
    for (const auto &klass : _builder->klasses())
    {
        class_struct(klass);
    }

    std::vector<llvm::Constant *> init_methods;

    // create null init method, because tag 0 is reserved
    // always have at least one class
    assert(_builder->klasses().size());
    auto *const init_method = _module.getFunction(_builder->klasses()[0]->init_method());
    init_methods.push_back(llvm::ConstantPointerNull::get(init_method->getType()));

    for (const auto &klass : _builder->klasses())
    {
        auto *const init_method = _module.getFunction(klass->init_method());
        GUARANTEE_DEBUG(init_method);
        init_methods.push_back(init_method);
    }

    GUARANTEE_DEBUG(init_methods.size());
    make_constant_array(_runtime.symbol_name(RuntimeLLVM::RuntimeLLVMSymbols::CLASS_OBJ_TAB),
                        llvm::ArrayType::get(init_methods[0]->getType(), init_methods.size()), init_methods);
}

void DataLLVM::gen_class_name_tab()
{
    std::vector<llvm::Constant *> names;

    for (const auto &klass : _builder->klasses())
    {
        names.push_back(string_const(klass->name()));
    }

    GUARANTEE_DEBUG(names.size());
    make_constant_array(
        _runtime.symbol_name(RuntimeLLVM::RuntimeLLVMSymbols::CLASS_NAME_TAB),
        llvm::ArrayType::get(class_struct(_builder->klass(BaseClassesNames[BaseClasses::STRING]))->getPointerTo(),
                             names.size()),
        names);
}

void DataLLVM::emit_inner(const std::string &out_file)
{
}