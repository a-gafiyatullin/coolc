#include "RuntimeMips.h"

using namespace codegen;

RuntimeMips::RuntimeMips()
    : Runtime(Label(static_cast<std::string>(CLASS_NAME_TAB)), Label(static_cast<std::string>(CLASS_OBJ_TAB))),
      _equality_test(static_cast<std::string>(EQUALITY_TEST), Label::LabelPolicy::ALLOW_NO_BIND),
      _object_copy(static_cast<std::string>(OBJECT_COPY), Label::LabelPolicy::ALLOW_NO_BIND),
      _case_abort(static_cast<std::string>(CASE_ABORT), Label::LabelPolicy::ALLOW_NO_BIND),
      _case_abort2(static_cast<std::string>(CASE_ABORT2), Label::LabelPolicy::ALLOW_NO_BIND),
      _dispatch_abort(static_cast<std::string>(DISPATCH_ABORT), Label::LabelPolicy::ALLOW_NO_BIND),
      _gen_gc_assign(static_cast<std::string>(GEN_GC_ASSIGN), Label::LabelPolicy::ALLOW_NO_BIND)
{
    _method_by_name.insert({static_cast<std::string>(EQUALITY_TEST), &_equality_test});
    _method_by_name.insert({static_cast<std::string>(OBJECT_COPY), &_object_copy});
    _method_by_name.insert({static_cast<std::string>(CASE_ABORT), &_case_abort});
    _method_by_name.insert({static_cast<std::string>(CASE_ABORT2), &_case_abort2});
    _method_by_name.insert({static_cast<std::string>(DISPATCH_ABORT), &_dispatch_abort});
    _method_by_name.insert({static_cast<std::string>(GEN_GC_ASSIGN), &_gen_gc_assign});
}