#include "Klass.h"

using namespace codegen;

Klass::Klass(const std::shared_ptr<ast::Class> &klass, const KlassBuilder &builer) : _klass(klass->_type)
{
    init_header();

    const auto &parent_class = builer._klasses.at(klass->_parent->_string);

    _fields.insert(_fields.end(), parent_class->fields_begin(), parent_class->fields_end());
    _methods.insert(_methods.end(), parent_class->methods_begin(), parent_class->methods_end());

    divide_features(klass->_features);

    CODEGEN_VERBOSE_ONLY(dump_fields());
    CODEGEN_VERBOSE_ONLY(dump_methods());
}

Klass::Klass() : _klass(nullptr)
{
    init_header();
}

void Klass::init_header()
{
    // add reference counter, tag and dispatch table
    for (int i = 0; i < HEADER_FIELDS; i++)
    {
        _fields.push_back(std::make_shared<ast::Feature>());
        _fields.back()->_base = std::move(ast::AttrFeature());
        _fields.back()->_type = semant::Semant::int_type();
    }
}

void Klass::divide_features(const std::vector<std::shared_ptr<ast::Feature>> &features)
{
    CODEGEN_VERBOSE_ONLY(LOG_ENTER("DIVIDE FEATURES:"));

    std::for_each(features.begin(), features.end(), [this](const auto &feature) {
        if (std::holds_alternative<ast::MethodFeature>(feature->_base))
        {
            auto table_entry = std::find_if(
                _methods.begin(), _methods.end(),
                [&feature](const std::pair<std::shared_ptr<ast::Type>, std::shared_ptr<ast::Feature>> &entry) {
                    return entry.second->_object->_object == feature->_object->_object;
                });

            if (table_entry == _methods.end())
            {
                CODEGEN_VERBOSE_ONLY(LOG("Adds method \"" + feature->_object->_object + "\""););
                _methods.push_back(std::make_pair(_klass, feature));
            }
            else
            {
                CODEGEN_VERBOSE_ONLY(LOG("Overload method \"" + feature->_object->_object + "\""););
                table_entry->first = _klass;
                table_entry->second = feature;
            }
        }
        else
        {
            CODEGEN_VERBOSE_ONLY(LOG("Adds field \"" + feature->_object->_object + "\""););
            _fields.push_back(feature);
        }
    });

    CODEGEN_VERBOSE_ONLY(LOG_EXIT("DIVIDE FEATURES."));
}

#ifdef CODEGEN_VERBOSE_ONLY
void Klass::dump_fields() const
{
    LOG_ENTER("ACTUAL FIELDS:");

    std::for_each(_fields.begin() + HEADER_FIELDS, _fields.end(),
                  [](const auto &f) { LOG(f->_object->_object + ": " + f->_type->_string); });

    LOG_EXIT("ACTUAL FIELDS.");
}

void Klass::dump_methods() const
{
    LOG_ENTER("ACTUAL METHODS:");

    std::for_each(_methods.begin(), _methods.end(), [](const auto &m) {
        LOG_NO_CR(m.second->_type->_string + " " + m.first->_string + "::" + m.second->_object->_object + "(");

        const auto &formals = std::get<ast::MethodFeature>(m.second->_base)._formals;
        std::for_each(formals.begin(), formals.end(),
                      [](const auto &f) { LOG_NO_CR_NO_IDENT(f->_object->_object + ": " + f->_type->_string + ","); });

        LOG_NO_IDENT(")");
    });

    LOG_EXIT("ACTUAL METHODS.");
}
#endif // CODEGEN_VERBOSE_ONLY

uint64_t Klass::method_offset(const std::string &method_name) const
{
    return (std::find_if(
                _methods.begin(), _methods.end(),
                [&method_name](const auto &method) { return method.second->_object->_object == method_name; }) -
            _methods.begin()) *
           WORD_SIZE;
}

std::string Klass::method_full_name(const std::string &method_name) const
{
    const auto &entry = std::find_if(_methods.begin(), _methods.end(), [&method_name](const auto &method) {
        return method.second->_object->_object == method_name;
    });

    return entry->first->_string + "." + entry->second->_object->_object;
}

std::string Klass::method_full_name(const int &n) const
{
    GUARANTEE_DEBUG(n < _methods.size());

    const auto &method = _methods[n];
    return method.first->_string + "." + method.second->_object->_object;
}

int KlassBuilder::build_klass(const std::shared_ptr<semant::ClassNode> &node, const int &tag)
{
    const auto &klass = node->_class;

    CODEGEN_VERBOSE_ONLY(LOG_ENTER("BUILD KLASS FOR " + klass->_type->_string + ":"));

    _klasses.insert(std::make_pair(klass->_type->_string, std::make_shared<Klass>(klass, *this)));

    int this_class_tag = tag;
    int child_max_tag = tag;
    std::for_each(node->_children.begin(), node->_children.end(),
                  [&](const auto &node) { child_max_tag = build_klass(node, child_max_tag + 1); });

    _klasses[klass->_type->_string]->set_tags(this_class_tag, child_max_tag);

    CODEGEN_VERBOSE_ONLY(
        LOG("Set tags: (" + std::to_string(this_class_tag) + ", " + std::to_string(child_max_tag) + ")"););

    CODEGEN_VERBOSE_ONLY(LOG_EXIT("BUILD KLASS FOR " + klass->_type->_string + "."));
    return child_max_tag;
}

KlassBuilder::KlassBuilder(const std::shared_ptr<semant::ClassNode> &root)
{
    CODEGEN_VERBOSE_ONLY(LOG_ENTER("KlassBuilder:"));

    _klasses.clear();

    // parent of the Object class. Need for algorithms
    _klasses.insert(std::make_pair(semant::Semant::empty_type()->_string, std::make_shared<Klass>()));

    build_klass(root, 0);

    // delete parent of the Object class
    _klasses.erase(semant::Semant::empty_type()->_string);

    // transform to vector and sort by tag
    _klasses_by_tag.resize(_klasses.size());
    transform(_klasses.begin(), _klasses.end(), _klasses_by_tag.begin(), [](const auto &pair) { return pair.second; });
    sort(_klasses_by_tag.begin(), _klasses_by_tag.end(),
         [](const auto &l, const auto &r) { return l->tag() < r->tag(); });

    CODEGEN_VERBOSE_ONLY(LOG_EXIT("KlassBuilder."));
}
