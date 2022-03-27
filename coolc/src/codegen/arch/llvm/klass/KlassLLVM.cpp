#include "KlassLLVM.h"

using namespace codegen;

std::string KlassLLVM::method_full_name(const std::string &method_name) const
{
    const auto &entry = std::find_if(_methods.begin(), _methods.end(), [&method_name](const auto &method) {
        return method.second->_object->_object == method_name;
    });

    return entry->first->_string + "_" + entry->second->_object->_object;
}

std::shared_ptr<Klass> KlassBuilderLLVM::create_klass(const std::shared_ptr<ast::Class> &klass)
{
    return klass ? std::make_shared<KlassLLVM>(klass, this) : std::make_shared<KlassLLVM>();
}