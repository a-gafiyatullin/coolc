#include "DataMyIR.hpp"
#include "codegen/arch/myir/ir/IR.inline.hpp"
#include "codegen/emitter/data/Data.inline.h"
#include "decls/Decls.h"

using namespace codegen;

DataMyIR::DataMyIR(const std::shared_ptr<KlassBuilder> &builder, myir::Module &module, const RuntimeMyIR &runtime)
    : Data(builder), _module(module), _runtime(runtime)
{
    // construct all dispatch tables and some global tables
    for (const auto &klass : _builder->klasses())
    {
        class_disp_tab(klass);
    }

    // need thod table initialized during ir construction
    gen_class_obj_tab_inner();
}

void DataMyIR::make_init_method(const std::shared_ptr<Klass> &klass)
{
    const auto init_method_name = klass->init_method();
    CODEGEN_VERBOSE_ONLY(LOG("Declare init method \"" + init_method_name + "\""));

    _module.add(
        new myir::Function(init_method_name, {new myir::Variable(SelfObject, myir::OperandType::POINTER)}, myir::VOID));
}

void DataMyIR::class_disp_tab_inner(const std::shared_ptr<Klass> &klass)
{
    // create init method in meanwhile
    make_init_method(klass);

    std::vector<myir::Operand *> methods;

    for_each(klass->methods_begin(), klass->methods_end(), [this, &methods, &klass](const auto &method) {
        auto method_full_name = klass->method_full_name(method.second->_object->_object);

        CODEGEN_VERBOSE_ONLY(LOG_ENTER("DECLARE METHOD \"" + method_full_name + "\""));

        const auto &return_type = method.second->_type;

        // maybe already have such a method? If so, just get a pointer to it, otherwise create a new one
        auto func = _module.get<myir::Function>(method_full_name);
        if (!func)
        {
            std::vector<myir::Variable *> args;
            args.push_back(new myir::Variable(SelfObject, myir::OperandType::POINTER)); // this

            // formals
            const auto &method_formals = std::get<ast::MethodFeature>(method.second->_base);
            for (const auto &formal : method_formals._formals)
            {
                const auto &formal_type = formal->_type->_string;
                CODEGEN_VERBOSE_ONLY(LOG("Formal of type \"" + formal_type + "\""));
                // all arguments are pointer
                args.push_back(new myir::Variable(formal->_object->_object, myir::OperandType::POINTER));
            }

            CODEGEN_VERBOSE_ONLY(LOG("Return type: \"" + return_type->_string + "\""));

            // cool methods always return pointer
            func = new myir::Function(method_full_name, args, myir::OperandType::POINTER);
            _module.add(func);

            CODEGEN_VERBOSE_ONLY(LOG_EXIT("DECLARE METHOD \"" + method_full_name + "\""));

            methods.push_back(func);
        }
    });

    const auto &disp_tab_name = klass->disp_tab();
    _module.add(new myir::GlobalConstant(disp_tab_name, methods, myir::STRUCTURE));

    _dispatch_tables[klass->name()] = _module.get<myir::GlobalConstant>(disp_tab_name);
}

void DataMyIR::int_const_inner(const int64_t &value)
{
    const auto &klass_name = BaseClassesNames[BaseClasses::INT];

    const auto &klass = _builder->klass(klass_name);

    auto const_name = Names::int_constant();

    _module.add(
        new myir::GlobalConstant(const_name,
                                 {new myir::Constant(MarkWordSetValue, _runtime.header_elem_type(HeaderLayout::Mark)),
                                  new myir::Constant(klass->tag(), _runtime.header_elem_type(HeaderLayout::Tag)),
                                  new myir::Constant(klass->size(), _runtime.header_elem_type(HeaderLayout::Size)),
                                  _dispatch_tables.at(klass_name), new myir::Constant(value, myir::OperandType::INT64)},
                                 myir::STRUCTURE));

    _int_constants.insert({value, _module.get<myir::GlobalConstant>(const_name)});
}

void DataMyIR::string_const_inner(const std::string &str)
{
    const auto &klass_name = BaseClassesNames[BaseClasses::STRING];
    const auto &klass = _builder->klass(klass_name);

    std::vector<myir::Operand *> elements;

    elements.push_back(new myir::Constant(MarkWordSetValue, _runtime.header_elem_type(HeaderLayout::Mark)));
    elements.push_back(new myir::Constant(klass->tag(), _runtime.header_elem_type(HeaderLayout::Tag)));
    elements.push_back(new myir::Constant(
        klass->size() + str.length() - (WORD_SIZE - 1),
        _runtime.header_elem_type(HeaderLayout::Size))); // native string is a 8 byte field, so substract 7 for '\0'
    elements.push_back(_dispatch_tables.at(klass_name));
    elements.push_back(int_const(str.length())); // length field

    // and now add string
    for (auto i = 0; i < str.size(); i++)
    {
        elements.push_back(new myir::Constant(str.at(i), myir::INT8));
    }

    elements.push_back(new myir::Constant(0, myir::INT8));

    auto string_name = Names::string_constant();
    _module.add(new myir::GlobalConstant(string_name, elements, myir::STRUCTURE));

    _string_constants.insert({str, _module.get<myir::GlobalConstant>(string_name)});
}

void DataMyIR::bool_const_inner(const bool &value)
{
    const auto &klass_name = BaseClassesNames[BaseClasses::BOOL];

    const auto &klass = _builder->klass(klass_name);

    auto const_name = Names::bool_constant();

    _module.add(
        new myir::GlobalConstant(const_name,
                                 {new myir::Constant(MarkWordSetValue, _runtime.header_elem_type(HeaderLayout::Mark)),
                                  new myir::Constant(klass->tag(), _runtime.header_elem_type(HeaderLayout::Tag)),
                                  new myir::Constant(klass->size(), _runtime.header_elem_type(HeaderLayout::Size)),
                                  _dispatch_tables.at(klass_name), new myir::Constant(value, myir::OperandType::INT64)},
                                 myir::STRUCTURE));

    _bool_constants.insert({value, _module.get<myir::GlobalConstant>(const_name)});
}

void DataMyIR::gen_class_obj_tab_inner()
{
    std::vector<myir::Operand *> init_methods;

    // create null init method, because tag 0 is reserved
    assert(_builder->klasses().size());
    init_methods.push_back(new myir::Constant(0, myir::POINTER));

    for (const auto &klass : _builder->klasses())
    {
        auto *init_method = _module.get<myir::Function>(klass->init_method());
        init_methods.push_back(init_method);
    }

    GUARANTEE_DEBUG(init_methods.size());
    _module.add(
        new myir::GlobalConstant(_runtime.symbol_name(RuntimeMyIR::CLASS_OBJ_TAB), init_methods, myir::STRUCTURE));
}

void DataMyIR::gen_class_name_tab()
{
    std::vector<myir::Operand *> names;

    for (const auto &klass : _builder->klasses())
    {
        names.push_back(string_const(klass->name()));
    }

    GUARANTEE_DEBUG(names.size());
    _module.add(new myir::GlobalConstant(_runtime.symbol_name(RuntimeMyIR::CLASS_NAME_TAB), names, myir::STRUCTURE));
}

void DataMyIR::emit_inner(const std::string &out_file) {}