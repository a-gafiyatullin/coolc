#include "DataMips.h"
#include "codegen/emitter/data/Data.inline.h"

using namespace codegen;

#define __ _asm.

void DataMips::string_const_inner(const std::string &str)
{
    const auto &size_label = int_const(str.length());
    _string_constants.insert({str, Label(NameConstructor::string_constant())});

    __ word(-1);

    const auto &string_klass = _builder->klass(BaseClassesNames[BaseClasses::STRING]);

    const AssemblerMarkSection mark(_asm, _string_constants.find(str)->second);
    __ word(string_klass->tag());
    __ word(string_klass->size() / WORD_SIZE +
            std::max(static_cast<int>(std::ceil(str.length() / (double)WORD_SIZE)), 1)); // 4 + str len
    __ word(Label(NameConstructor::disp_table(string_klass->name())));
    __ word(size_label);
    __ encode_string(str);
    __ byte(0);  // \0
    __ align(2); // align to 4 bytes next data
}

void DataMips::bool_const_inner(const bool &value)
{
    _bool_constants.insert({value, Label(NameConstructor::bool_constant())});

    __ word(-1);

    const auto &bool_klass = _builder->klass(BaseClassesNames[BaseClasses::BOOL]);

    const AssemblerMarkSection mark(_asm, _bool_constants.find(value)->second);
    __ word(bool_klass->tag());
    __ word(bool_klass->size());
    __ word(Label(NameConstructor::disp_table(bool_klass->name())));
    __ word(value ? TrueVal : FalseVal);
}

void DataMips::int_const_inner(const int64_t &value)
{
    _int_constants.insert({value, Label(NameConstructor::int_constant())});

    __ word(-1);

    const auto &int_klass = _builder->klass(BaseClassesNames[BaseClasses::INT]);

    const AssemblerMarkSection mark(_asm, _int_constants.find(value)->second);
    __ word(int_klass->tag());
    __ word(int_klass->size());
    __ word(Label(NameConstructor::disp_table(int_klass->name())));
    __ word(value);
}

// A table, which at index (class tag) * WORD_SIZE contains a pointer Data
// with the class tag to a String object containing the name of the class associated
void DataMips::gen_class_name_tab()
{
    // declare all consts
    for (const auto &klass : _builder->klasses())
    {
        string_const(klass->name());
    }

    const AssemblerMarkSection mark(_asm, _runtime.class_name_tab());

    // gather to table
    for (const auto &klass : _builder->klasses())
    {
        __ word(string_const(klass->name()));
    }
}

void DataMips::gen_class_obj_tab()
{
    gen_prototypes();
    gen_dispatch_tabs();

    const AssemblerMarkSection mark(_asm, _runtime.class_obj_tab());

    for (const auto &klass : _builder->klasses())
    {
        const auto &class_name = klass->name();
        __ word(Label(NameConstructor::prototype(class_name)));
        __ word(Label(NameConstructor::init_method(class_name)));
    }
}

void DataMips::gen_prototypes()
{
    int_const(0);
    string_const("");

    for (const auto &klass_iter : *_builder)
    {
        class_struct(klass_iter.second);
    }
}

void DataMips::gen_dispatch_tabs()
{
    for (const auto &klass_iter : *_builder)
    {
        class_disp_tab(klass_iter.second);
    }
}

DataMips::DataMips(const std::shared_ptr<KlassBuilder> &builder, const RuntimeMips &runtime)
    : Data(builder), _asm(_code), _runtime(runtime)
{
    const Label int_tag(static_cast<std::string>(RuntimeMips::INT_TAG_NAME));
    const Label bool_tag(static_cast<std::string>(RuntimeMips::BOOL_TAG_NAME));
    const Label string_tag(static_cast<std::string>(RuntimeMips::STRING_TAG_NAME));

    __ data_section();
    __ align(2);

    __ global(_runtime.class_name_tab());
    __ global(Label(NameConstructor::prototype(MainClassName)));
    __ global(Label(NameConstructor::prototype(BaseClassesNames[BaseClasses::INT])));
    __ global(Label(NameConstructor::prototype(BaseClassesNames[BaseClasses::STRING])));
    __ global(Label(NameConstructor::bool_constant())); // false
    __ global(Label(NameConstructor::bool_constant())); // true
    __ global(int_tag);
    __ global(bool_tag);
    __ global(string_tag);

    // now create tags for basic types
    {
        const AssemblerMarkSection mark(_asm, int_tag);
        __ word(_builder->tag(BaseClassesNames[BaseClasses::INT]));
    }

    {
        const AssemblerMarkSection mark(_asm, bool_tag);
        __ word(_builder->tag(BaseClassesNames[BaseClasses::BOOL]));
    }

    {
        const AssemblerMarkSection mark(_asm, string_tag);
        __ word(_builder->tag(BaseClassesNames[BaseClasses::STRING]));
    }

    // Generational GC Interface
    const Label memmrg_init(static_cast<std::string>(RuntimeMips::MEM_MGR_INIT));
    __ global(memmrg_init);

    const Label memmrg_collector(static_cast<std::string>(RuntimeMips::MEM_MGR_COLLECTOR));
    {
        const AssemblerMarkSection mark(_asm, memmrg_init);
        __ word(Label(static_cast<std::string>(RuntimeMips::GEN_GC_INIT), Label::ALLOW_NO_BIND)); // external symbol
        __ global(memmrg_collector);
    }

    const Label memmrg_test(static_cast<std::string>(RuntimeMips::MEM_MGR_TEST));
    {
        const AssemblerMarkSection mark(_asm, memmrg_collector);
        __ word(Label(static_cast<std::string>(RuntimeMips::GEN_GC_COLLECT), Label::ALLOW_NO_BIND)); // external symbol
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

    bool_const(false);
    bool_const(true);
}

void DataMips::emit_inner(const std::string &out_file_name)
{
    std::ofstream out_file(out_file_name);

    const Label heap_start_label(static_cast<std::string>(RuntimeMips::HEAP_START));
    __ global(heap_start_label);
    AssemblerMarkSection(_asm, heap_start_label);
    __ word(0);

    out_file << static_cast<std::string>(_code);

    out_file.close();
}

void DataMips::class_struct_inner(const std::shared_ptr<Klass> &klass)
{
    const auto &class_name = klass->name();

    _classes.insert({class_name, Label(NameConstructor::prototype(class_name))});

    __ word(-1);

    const AssemblerMarkSection mark(_asm, _classes.find(class_name)->second);

    __ word(klass->tag());                                   // tag
    __ word(klass->size() / WORD_SIZE);                      // size in words
    __ word(Label(NameConstructor::disp_table(class_name))); // pointer to dispatch table

    // set all fields to void
    std::for_each(klass->fields_begin(), klass->fields_end(), [&](const auto &field) {
        if (!semant::Semant::is_trivial_type(field->_type))
        {
            __ word(0);
        }
        else
        {
            __ word(init_value(field->_type));
        }
    });
}

void DataMips::class_disp_tab_inner(const std::shared_ptr<Klass> &klass)
{
    const auto &class_name = klass->name();

    _dispatch_tables.insert({class_name, Label(NameConstructor::disp_table(class_name))});

    const AssemblerMarkSection mark(_asm, _dispatch_tables.find(class_name)->second);
    const auto &mips_klass = std::static_pointer_cast<KlassMips>(klass);

    for (auto i = 0; i < mips_klass->methods_num(); i++)
    {
        __ word(Label(mips_klass->method_full_name(i)));
    }
}