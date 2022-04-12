#include "KlassMips.h"

using namespace codegen;

std::string KlassMips::method_full_name(const std::string &method_name) const
{
    const auto &entry = std::find_if(_methods.begin(), _methods.end(), [&method_name](const auto &method) {
        return method.second->_object->_object == method_name;
    });

    return NameConstructor::method_full_name(entry->first->_string, entry->second->_object->_object, FULL_METHOD_DELIM);
}

std::string KlassMips::method_full_name(const int &n) const
{
    GUARANTEE_DEBUG(n < _methods.size());

    const auto &method = _methods[n];
    return NameConstructor::method_full_name(method.first->_string, method.second->_object->_object, FULL_METHOD_DELIM);
}

std::shared_ptr<Klass> KlassBuilderMips::create_klass(const std::shared_ptr<ast::Class> &klass)
{
    return klass ? std::make_shared<KlassMips>(klass, this) : std::make_shared<KlassMips>();
}