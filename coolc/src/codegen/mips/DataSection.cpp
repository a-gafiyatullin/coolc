#include "codegen/mips/DataSection.h"

using namespace codegen;

#define __ _asm.

const Label &DataSection::declare_string_const(const std::string &str)
{
    // maybe we already have such a constant
    if (_string_constants.find(str) == _string_constants.end())
    {
        const Label &size_label = declare_int_const(str.length());
        _string_constants.insert({str, Label(_asm, static_cast<std::string>(CONST_STRING_PREFIX) +
                                                       std::to_string(_string_constants.size()))});

        __ word(-1);

        const auto &string_klass = _builder.klass(semant::Semant::string_type()->_string);

        const AssemblerMarkSection mark(_asm, _string_constants.find(str)->second);
        __ word(string_klass->tag());
        __ word(STRING_CONST_BASE_SIZE_IN_WORDS +
                std::max(static_cast<int>(std::ceil(str.length() / (double)WORD_SIZE)), 1)); // 4 + str len
        __ word(Label(_asm, NameSpace::disp_table(string_klass)));
        __ word(size_label);
        __ encode_string(str);
        __ byte(0);  // \0
        __ align(2); // align to 4 bytes next data
    }

    return _string_constants.at(str);
}

const Label &DataSection::declare_bool_const(const bool &value)
{
    // maybe we already have such a constant
    if (_bool_constants.find(value) == _bool_constants.end())
    {
        _bool_constants.insert(std::make_pair(
            value, Label(_asm, static_cast<std::string>(CONST_BOOL_PREFIX) + std::to_string(_bool_constants.size()))));

        __ word(-1);

        const auto &bool_klass = _builder.klass(semant::Semant::bool_type()->_string);

        const AssemblerMarkSection mark(_asm, _bool_constants.find(value)->second);
        __ word(bool_klass->tag());
        __ word(BOOL_CONST_SIZE_IN_WORDS);
        __ word(Label(_asm, NameSpace::disp_table(bool_klass)));
        __ word(value ? true_value() : false_value());
    }

    return _bool_constants.find(value)->second;
}

const Label &DataSection::declare_int_const(const int32_t &value)
{
    // maybe we already have such a constant
    if (_int_constants.find(value) == _int_constants.end())
    {
        _int_constants.insert(std::make_pair(
            value, Label(_asm, static_cast<std::string>(CONST_INT_PREFIX) + std::to_string(_int_constants.size()))));

        __ word(-1);

        const auto &int_klass = _builder.klass(semant::Semant::int_type()->_string);

        const AssemblerMarkSection mark(_asm, _int_constants.find(value)->second);
        __ word(int_klass->tag());
        __ word(INT_CONST_SIZE_IN_WORDS);
        __ word(Label(_asm, NameSpace::disp_table(int_klass)));
        __ word(value);
    }

    return _int_constants.find(value)->second;
}

// A table, which at index (class tag) * WORD_SIZE contains a pointer Data
// with the class tag
// to a String object containing the name of the class associated
void DataSection::get_class_name_tab()
{
    // declare all consts
    for (const auto &klass : _builder.klasses())
    {
        declare_string_const(klass->name());
    }

    const AssemblerMarkSection mark(_asm, Label(_asm, "class_nameTab"));

    // gather to table
    for (const auto &klass : _builder.klasses())
    {
        __ word(declare_string_const(klass->name()));
    }
}

void DataSection::get_class_obj_tab()
{
    const AssemblerMarkSection mark(_asm, _class_obj_tab);

    for (const auto &klass : _builder.klasses())
    {
        __ word(Label(_asm, NameSpace::prototype(klass)));
        __ word(Label(_asm, NameSpace::init_method(klass)));
    }
}

void DataSection::get_all_prototypes()
{
    std::for_each(_builder.begin(), _builder.end(), [&](const auto &klass_iter) {
        __ word(-1);

        const auto &klass = klass_iter.second;
        const AssemblerMarkSection mark(_asm, Label(_asm, NameSpace::prototype(klass)));

        __ word(klass->tag());                              // tag
        __ word(klass->size() / WORD_SIZE);                 // size in words
        __ word(Label(_asm, NameSpace::disp_table(klass))); // pointer to dispatch table

        // set all fields to void
        std::for_each(klass->fields_begin(), klass->fields_end(), [&](const auto &field) {
            if (!semant::Semant::is_trivial_type(field->_type))
            {
                __ word(0);
            }
            else
            {
                __ word(emit_init_value(field->_type));
            }
        });
    });
}

void DataSection::get_all_dispatch_tab()
{
    std::for_each(_builder.begin(), _builder.end(), [&](const auto &klass_iter) {
        const auto &klass = klass_iter.second;

        const AssemblerMarkSection mark(_asm, Label(_asm, NameSpace::disp_table(klass)));
        for (int i = 0; i < klass->methods_num(); i++)
        {
            __ word(Label(_asm, klass->method_full_name(i)));
        }
    });
}

DataSection::DataSection(const KlassBuilder &builder)
    : _asm(_code), _builder(builder), _class_obj_tab(_asm, "class_objTab")
{
    const Label class_name_tab(_asm, "class_nameTab");
    const Label main_prot_obj(_asm, "Main_protObj");
    const Label int_prot_obj(_asm, "Int_protObj");
    const Label string_prot_obj(_asm, "String_protObj");
    const Label bool_const0(_asm, "bool_const0");
    const Label bool_const1(_asm, "bool_const1");

    const Label int_tag(_asm, "_int_tag");
    const Label bool_tag(_asm, "_bool_tag");
    const Label string_tag(_asm, "_string_tag");

    __ data_section();
    __ align(2);

    __ global(class_name_tab);
    __ global(main_prot_obj);
    __ global(int_prot_obj);
    __ global(string_prot_obj);
    __ global(bool_const0);
    __ global(bool_const1);
    __ global(int_tag);
    __ global(bool_tag);
    __ global(string_tag);

    // now create tags for basic types
    {
        const AssemblerMarkSection mark(_asm, int_tag);
        __ word(_builder.tag(semant::Semant::int_type()->_string));
    }

    {
        const AssemblerMarkSection mark(_asm, bool_tag);
        __ word(_builder.tag(semant::Semant::bool_type()->_string));
    }

    {
        const AssemblerMarkSection mark(_asm, string_tag);
        __ word(_builder.tag(semant::Semant::string_type()->_string));
    }

    // Generational GC Interface
    const Label memmrg_init(_asm, "_MemMgr_INITIALIZER");
    __ global(memmrg_init);

    const Label memmrg_collector(_asm, "_MemMgr_COLLECTOR");
    {
        const AssemblerMarkSection mark(_asm, memmrg_init);
        __ word(Label(_asm, "_GenGC_Init", Label::ALLOW_NO_BIND)); // external symbol
        __ global(memmrg_collector);
    }

    const Label memmrg_test(_asm, "_MemMgr_TEST");
    {
        const AssemblerMarkSection mark(_asm, memmrg_collector);
        __ word(Label(_asm, "_GenGC_Collect", Label::ALLOW_NO_BIND)); // external symbol
        __ global(memmrg_test);
    }

    {
        const AssemblerMarkSection mark(_asm, memmrg_test);
#ifdef CODEGEN_GC_STRESS_TEST
        __ word(-1);
#else
        __ word(0);
#endif // CODEGEN_GC_STRESS_TEST
    }

    declare_bool_const(false);
    declare_bool_const(true);
}

CodeBuffer DataSection::emit()
{
    // set necessary constants for prototypes
    declare_int_const(0);
    declare_string_const("");

    get_all_prototypes();
    get_all_dispatch_tab();
    get_class_obj_tab();
    get_class_name_tab();

    const Label heap_start_label(_asm, "heap_start");
    __ global(heap_start_label);
    AssemblerMarkSection(_asm, heap_start_label);
    __ word(0);

    return _code;
}

const Label &DataSection::emit_init_value(const std::shared_ptr<ast::Type> &type)
{
    if (semant::Semant::is_string(type))
    {
        return declare_string_const("");
    }
    if (semant::Semant::is_int(type))
    {
        return declare_int_const(0);
    }
    if (semant::Semant::is_bool(type))
    {
        return declare_bool_const(false);
    }

    SHOULD_NOT_REACH_HERE();
}