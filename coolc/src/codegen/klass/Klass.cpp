#include "Klass.h"

using namespace codegen;

Klass::Klass(const std::shared_ptr<ast::Class> &klass, const KlassBuilder *builder)
    : _klass(klass->_type), _parent_klass(builder->_klasses.at(klass->_parent->_string))
{
    _fields.insert(_fields.end(), _parent_klass->fields_begin(), _parent_klass->fields_end());
    _methods.insert(_methods.end(), _parent_klass->methods_begin(), _parent_klass->methods_end());

    divide_features(klass->_features);

    CODEGEN_VERBOSE_ONLY(dump_fields());
    CODEGEN_VERBOSE_ONLY(dump_methods());
}

Klass::Klass() : _klass(nullptr), _parent_klass(nullptr)
{
}

void Klass::divide_features(const std::vector<std::shared_ptr<ast::Feature>> &features)
{
    CODEGEN_VERBOSE_ONLY(LOG_ENTER("DIVIDE FEATURES."));

    for (const auto &feature : features)
    {
        const auto &name = feature->_object->_object;

        if (std::holds_alternative<ast::MethodFeature>(feature->_base))
        {
            auto table_entry = std::find_if(_methods.begin(), _methods.end(), [&name](const auto &entry) {
                return entry.second->_object->_object == name;
            });

            if (table_entry == _methods.end())
            {
                CODEGEN_VERBOSE_ONLY(LOG("Adds method \"" + name + "\""););
                _methods.push_back(std::make_pair(_klass, feature));
            }
            else
            {
                CODEGEN_VERBOSE_ONLY(LOG("Overload method \"" + name + "\""););
                table_entry->first = _klass;
                table_entry->second = feature;
            }
        }
        else
        {
            CODEGEN_VERBOSE_ONLY(LOG("Adds field \"" + name + "\""););
            _fields.push_back(feature);
        }
    }

    CODEGEN_VERBOSE_ONLY(LOG_EXIT("DIVIDE FEATURES."));
}

#ifdef CODEGEN_VERBOSE_ONLY
void Klass::dump_fields() const
{
    LOG_ENTER("ACTUAL FIELDS.");

    for (const auto &f : _fields)
    {
        LOG(f->_object->_object + ": " + f->_type->_string);
    }

    LOG_EXIT("ACTUAL FIELDS.");
}

void Klass::dump_methods() const
{
    LOG_ENTER("ACTUAL METHODS.");

    for (const auto &m : _methods)
    {
        LOG_NO_CR(m.first->_string + "::" + m.second->_object->_object + "(");

        const auto &formals = std::get<ast::MethodFeature>(m.second->_base)._formals;
        auto n = 0;
        const auto formals_num = formals.size();
        for (const auto &f : formals)
        {
            n++;
            LOG_NO_CR_NO_IDENT(f->_object->_object + ": " + f->_type->_string + (n != formals_num ? ", " : ""));
        }

        LOG_NO_IDENT(") -> " + m.second->_type->_string);
    }

    LOG_EXIT("ACTUAL METHODS.");
}
#endif // CODEGEN_VERBOSE_ONLY

int KlassBuilder::build_klass(const std::shared_ptr<semant::ClassNode> &node, const int &tag)
{
    const auto &klass = node->_class;

    CODEGEN_VERBOSE_ONLY(LOG_ENTER("BUILD KLASS FOR \"" + klass->_type->_string + "\""));

    _klasses.insert({klass->_type->_string, make_klass(klass)});

    auto child_max_tag = tag;
    for (const auto &node : node->_children)
    {
        child_max_tag = build_klass(node, child_max_tag + 1);
    }

    _klasses[klass->_type->_string]->set_tags(tag, child_max_tag);

    CODEGEN_VERBOSE_ONLY(LOG("Set tags: (" + std::to_string(tag) + ", " + std::to_string(child_max_tag) + ")"););

    CODEGEN_VERBOSE_ONLY(LOG_EXIT("BUILD KLASS FOR \"" + klass->_type->_string + "\""));
    return child_max_tag;
}

size_t Klass::method_index(const std::string &method_name) const
{
    return std::find_if(_methods.begin(), _methods.end(),
                        [&method_name](const auto &method) { return method.second->_object->_object == method_name; }) -
           _methods.begin();
}

KlassBuilder::KlassBuilder(const std::shared_ptr<semant::ClassNode> &root) : _root(root)
{
}

void KlassBuilder::init()
{
    CODEGEN_VERBOSE_ONLY(LOG_ENTER("KlassBuilder."));

    _klasses.clear();

    // parent of the Object class. Need for algorithms
    _klasses.insert({semant::Semant::empty_type()->_string, make_klass(nullptr)});

    build_klass(_root, 1); // convention with GC: tag 0 is reserved

    // delete parent of the Object class
    _klasses.erase(semant::Semant::empty_type()->_string);

    // transform to vector and sort by tag
    _klasses_by_tag.resize(_klasses.size());
    transform(_klasses.begin(), _klasses.end(), _klasses_by_tag.begin(), [](const auto &pair) { return pair.second; });
    sort(_klasses_by_tag.begin(), _klasses_by_tag.end(),
         [](const auto &l, const auto &r) { return l->tag() < r->tag(); });

    CODEGEN_VERBOSE_ONLY(LOG_EXIT("KlassBuilder."));
}