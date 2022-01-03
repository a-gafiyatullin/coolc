#include "codegen/stack_machine/mips/spim/DataSection.h"

using namespace codegen;

#define __ _asm.

// ------------------------------------------ ClassCode ------------------------------------------
int ClassCode::get_method_index(const std::string &method_name) const
{
    return std::find_if(_disp_table.begin(), _disp_table.end(),
                        [&method_name](const auto &method)
                        {
                            return method.substr(method.find('.') + 1) == method_name;
                        }) -
           _disp_table.begin();
}

std::string ClassCode::get_method_full_name(const std::string &method_name) const
{
    return *std::find_if(_disp_table.begin(), _disp_table.end(),
                         [&method_name](const auto &method)
                         {
                             return method.substr(method.find('.') + 1) == method_name;
                         });
}

void ClassCode::set_disp_table_entry(const std::string &method_name)
{
    std::string full_name = std::move(DataSection::get_full_method_name(_class->_string, method_name));

    auto table_entry = std::find_if(_disp_table.begin(), _disp_table.end(),
                                    [&method_name](const std::string &entry)
                                    { return entry.substr(entry.find('.') + 1) == method_name; });

    if (table_entry == _disp_table.end())
    {
        CODEGEN_FULL_VERBOSE_ONLY(_logger.log("Adds method " + full_name + " to class " + _class->_string););
        _disp_table.push_back(std::move(full_name));
    }
    else
    {
        CODEGEN_FULL_VERBOSE_ONLY(_logger.log("Changes method " + *table_entry + " on method " + full_name););
        *table_entry = std::move(full_name);
    }
}

void ClassCode::construct_disp_table() const
{
    Assembler &_asm = _data.get_asm();

    const AssemblerMarkSection mark(_asm, Label(_asm, DataSection::get_disp_table_name(_class->_string)));

    std::for_each(_disp_table.begin(), _disp_table.end(),
                  [&_asm](const auto &entry)
                  {
                      __ word(Label(_asm, entry));
                  });
}

void ClassCode::construct_prototype() const
{
    Assembler &_asm = _data.get_asm();

    const AssemblerMarkSection mark(_asm, Label(_asm, DataSection::get_prototype_name(_class->_string)));

    __ word(_data.get_tag(_class->_string));                                 // tag
    __ word(_fields_types.size() + _OBJECT_BASE_SIZE_IN_WORDS);              // size in words
    __ word(Label(_asm, DataSection::get_disp_table_name(_class->_string))); // pointer to dispatch table

    // set all fields to void
    std::for_each(_fields_types.begin(), _fields_types.end(),
                  [&](const auto &type)
                  {
                      if (type == nullptr || !semant::Semant::is_trivial_type(type)) // some kind of dummy
                      {
                          __ word(0);
                      }
                      else
                      {
                          __ word(DataSection::emit_init_value(_asm, _data, type));
                      }
                  });
}

void ClassCode::inherit_disp_table(const ClassCode &code)
{
    _disp_table = code._disp_table;

#ifdef CODEGEN_FULL_VERBOSE
    _logger.log(_class->_string + " inherits disp table from " + code._class->_string + ":");
    std::for_each(_disp_table.begin(), _disp_table.end(), [this](const std::string &str)
                  { this->_logger.log(str); });
#endif // CODEGEN_FULL_VERBOSE
}

void ClassCode::inherit_fields(const ClassCode &code)
{
    _fields_types = code._fields_types;

#ifdef CODEGEN_FULL_VERBOSE
    _logger.log(_class->_string + " inherits fields from " + code._class->_string + ":");
    std::for_each(_fields_types.begin(), _fields_types.end(), [this](const auto &type)
                  { this->_logger.log(type == nullptr ? "nullptr" : type->_string); });
#endif // CODEGEN_FULL_VERBOSE
}

// ------------------------------------------ DataSection ------------------------------------------
int DataSection::get_tag(const std::string &type) const
{
    assert(_tags.find(type) != _tags.end());
    return _tags.at(type).first;
}

int DataSection::get_max_child_tag(const std::string &type) const
{
    assert(_tags.find(type) != _tags.end());
    return _tags.at(type).second;
}

int DataSection::create_tag(const std::string &type)
{
    const auto tag = _tags.find(type);
    if (tag != _tags.end())
    {
        return tag->second.first;
    }

    _tags.insert({type, {_tags.size(), 0}});
    return _tags.at(type).first;
}

const Label &DataSection::declare_string_const(const std::string &str)
{
    // maybe we already have such a constant
    if (_string_constants.find(str) == _string_constants.end())
    {
        const Label &size_label = declare_int_const(str.length());
        _string_constants.insert({str, Label(_asm, static_cast<std::string>(_CONST_STRING_PREFIX) +
                                                       std::to_string(_string_constants.size()))});

        __ word(-1);

        const AssemblerMarkSection mark(_asm, _string_constants.find(str)->second);
        __ word(get_string_tag());
        __ word(_STRING_CONST_BASE_SIZE_IN_WORDS + std::max(static_cast<int>(std::ceil(str.length() / 4.0)), 1)); // 4 + str len in bytes
        __ word(Label(_asm, DataSection::get_disp_table_name(static_cast<std::string>(_STRING_TYPE))));
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
        _bool_constants.insert(std::make_pair(value, Label(_asm, static_cast<std::string>(_CONST_BOOL_PREFIX) +
                                                                     std::to_string(_bool_constants.size()))));

        __ word(-1);

        const AssemblerMarkSection mark(_asm, _bool_constants.find(value)->second);
        __ word(get_bool_tag());
        __ word(_BOOL_CONST_SIZE_IN_WORDS);
        __ word(Label(_asm, DataSection::get_disp_table_name(static_cast<std::string>(_BOOL_TYPE))));
        __ word(value ? get_true_value() : get_false_value());
    }

    return _bool_constants.find(value)->second;
}

const Label &DataSection::declare_int_const(const int32_t &value)
{
    // maybe we already have such a constant
    if (_int_constants.find(value) == _int_constants.end())
    {
        _int_constants.insert(std::make_pair(value, Label(_asm, static_cast<std::string>(_CONST_INT_PREFIX) +
                                                                    std::to_string(_int_constants.size()))));

        __ word(-1);

        const AssemblerMarkSection mark(_asm, _int_constants.find(value)->second);
        __ word(get_int_tag());
        __ word(_INT_CONST_SIZE_IN_WORDS);
        __ word(Label(_asm, DataSection::get_disp_table_name(static_cast<std::string>(_INT_TYPE))));
        __ word(value);
    }

    return _int_constants.find(value)->second;
}

ClassCode &DataSection::create_class_code(const std::shared_ptr<ast::Type> &class_)
{
    _class_codes.insert(std::make_pair(create_tag(class_->_string),
                                       std::move(ClassCode(class_, declare_string_const(class_->_string), *this))));
    return _class_codes.find(get_tag(class_->_string))->second;
}

ClassCode &DataSection::get_class_code(const std::shared_ptr<ast::Type> &class_)
{
    CODEGEN_FULL_VERBOSE_ONLY(assert(_class_codes.find(get_tag(class_->_string)) != _class_codes.end()));
    return _class_codes.find(get_tag(class_->_string))->second;
}

// A table, which at index (class tag) * 4 contains a pointer Data
// to a String object containing the name of the class associated
// with the class tag
void DataSection::get_class_name_tab()
{
    const AssemblerMarkSection mark(_asm, Label(_asm, "class_nameTab"));

    for (int i = 0; i < _class_codes.size(); i++)
    {
        __ word(_class_codes.at(i)._class_name_const);
    }
}

void DataSection::get_class_obj_tab()
{
    const AssemblerMarkSection mark(_asm, _class_obj_tab);

    for (int i = 0; i < _class_codes.size(); i++)
    {
        __ word(Label(_asm, DataSection::get_prototype_name(_class_codes.at(i)._class->_string)));
        __ word(Label(_asm, DataSection::get_init_method_name(_class_codes.at(i)._class->_string)));
    }
}

void DataSection::get_all_prototypes()
{
    std::for_each(_class_codes.begin(), _class_codes.end(),
                  [&](const auto &code)
                  {
                      __ word(-1);
                      code.second.construct_prototype();
                  });
}

void DataSection::get_all_dispatch_tab()
{
    std::for_each(_class_codes.begin(), _class_codes.end(),
                  [](const auto &code)
                  {
                      code.second.construct_disp_table();
                  });
}

std::string DataSection::get_full_method_name(const std::string &class_name, const std::string &method)
{
    return class_name + "." + method;
}

DataSection::DataSection(const std::shared_ptr<semant::ClassNode> &root)
    : _asm(_code),
      _class_obj_tab(_asm, "class_objTab")
{
    const Label class_nameTab(_asm, "class_nameTab");
    const Label main_protObj(_asm, "Main_protObj");
    const Label int_protObj(_asm, "Int_protObj");
    const Label string_protObj(_asm, "String_protObj");
    const Label bool_const0(_asm, "bool_const0");
    const Label bool_const1(_asm, "bool_const1");

    const Label _int_tag(_asm, "_int_tag");
    const Label _bool_tag(_asm, "_bool_tag");
    const Label _string_tag(_asm, "_string_tag");

    CODEGEN_FULL_VERBOSE_ONLY(_asm.set_parent_logger(&_logger););

    __ data_section();
    __ align(2);

    __ global(class_nameTab);
    __ global(main_protObj);
    __ global(int_protObj);
    __ global(string_protObj);
    __ global(bool_const0);
    __ global(bool_const1);
    __ global(_int_tag);
    __ global(_bool_tag);
    __ global(_string_tag);

    // have to create Object tag before any other types for correct CaseExpression emitting
    create_tag(root->_class->_type->_string);

    // now create tags for basic types
    {
        const AssemblerMarkSection mark(_asm, _int_tag);
        __ word(create_tag(static_cast<std::string>(_INT_TYPE)));
    }

    {
        const AssemblerMarkSection mark(_asm, _bool_tag);
        __ word(create_tag(static_cast<std::string>(_BOOL_TYPE)));
    }

    {
        const AssemblerMarkSection mark(_asm, _string_tag);
        __ word(create_tag(static_cast<std::string>(_STRING_TYPE)));
    }

    // let Object be Object parent. It's ok for now
    root->_class->_parent = root->_class->_type;
    build_class_info(root);

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
#endif //CODEGEN_GC_STRESS_TEST
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

const Label &DataSection::emit_init_value(Assembler &_asm, DataSection &_data, const std::shared_ptr<ast::Type> &type)
{
    if (semant::Semant::is_string(type))
    {
        return _data.declare_string_const("");
    }
    if (semant::Semant::is_int(type))
    {
        return _data.declare_int_const(0);
    }
    if (semant::Semant::is_bool(type))
    {
        return _data.declare_bool_const(false);
    }
}

int DataSection::build_class_info(const std::shared_ptr<semant::ClassNode> &node)
{
    const auto &class_ = node->_class;
    auto &code = create_class_code(class_->_type);

    CODEGEN_FULL_VERBOSE_ONLY(code.set_parent_logger(&_logger););

    // inherit dispatch table from parent
    code.inherit_disp_table(get_class_code(class_->_parent));

    std::for_each(class_->_features.begin(), class_->_features.end(),
                  [&code](const auto &feature)
                  {
                      if (std::holds_alternative<ast::MethodFeature>(feature->_base))
                      {
                          // add entry to dispatch table
                          code.set_disp_table_entry(feature->_object->_object);
                      }
                  });

    // gen info for childs and get max child tag
    int max_child_tag = get_tag(class_->_type->_string);
    std::for_each(node->_children.begin(), node->_children.end(), [&](const auto &node)
                  { max_child_tag = std::max(max_child_tag, build_class_info(node)); });

    _tags.at(class_->_type->_string).second = max_child_tag;
    CODEGEN_FULL_VERBOSE_ONLY(_logger.log("For class " + class_->_type->_string + " with tag " + std::to_string(_tags.at(class_->_type->_string).first) +
                                          " last child has tag " + std::to_string(max_child_tag)));

    return max_child_tag;
}