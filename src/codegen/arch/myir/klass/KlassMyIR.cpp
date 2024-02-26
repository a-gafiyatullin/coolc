#include "KlassMyIR.hpp"

using namespace codegen;

std::string KlassMyIR::method_full_name(const std::string &method_name) const
{
    const auto &entry = std::find_if(_methods.begin(), _methods.end(), [&method_name](const auto &method) {
        return method.second->_object->_object == method_name;
    });

    return Names::method_full_name(entry->first->_string, entry->second->_object->_object, FULL_METHOD_DELIM);
}

std::shared_ptr<Klass> KlassBuilderMyIR::make_klass(const std::shared_ptr<ast::Class> &klass)
{
    return klass ? std::make_shared<KlassMyIR>(klass, this) : std::make_shared<KlassMyIR>();
}
