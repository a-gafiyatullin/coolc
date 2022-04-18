#include "RuntimeMips.h"

using namespace codegen;

RuntimeMips::RuntimeMips()
    : _class_name_tab(SYMBOLS[CLASS_NAME_TAB]), _class_obj_tab(SYMBOLS[CLASS_OBJ_TAB]),
      _equality_test(SYMBOLS[EQUALITY_TEST], Label::LabelPolicy::ALLOW_NO_BIND),
      _object_copy(SYMBOLS[OBJECT_COPY], Label::LabelPolicy::ALLOW_NO_BIND),
      _case_abort(SYMBOLS[CASE_ABORT], Label::LabelPolicy::ALLOW_NO_BIND),
      _case_abort2(SYMBOLS[CASE_ABORT2], Label::LabelPolicy::ALLOW_NO_BIND),
      _dispatch_abort(SYMBOLS[DISPATCH_ABORT], Label::LabelPolicy::ALLOW_NO_BIND),
      _gen_gc_assign(SYMBOLS[GEN_GC_ASSIGN], Label::LabelPolicy::ALLOW_NO_BIND)
{
    _symbol_by_name.insert({SYMBOLS[EQUALITY_TEST], &_equality_test});
    _symbol_by_name.insert({SYMBOLS[OBJECT_COPY], &_object_copy});
    _symbol_by_name.insert({SYMBOLS[CASE_ABORT], &_case_abort});
    _symbol_by_name.insert({SYMBOLS[CASE_ABORT2], &_case_abort2});
    _symbol_by_name.insert({SYMBOLS[DISPATCH_ABORT], &_dispatch_abort});
    _symbol_by_name.insert({SYMBOLS[GEN_GC_ASSIGN], &_gen_gc_assign});

    _symbol_by_name.insert({SYMBOLS[CLASS_NAME_TAB], &_class_name_tab});
    _symbol_by_name.insert({SYMBOLS[CLASS_OBJ_TAB], &_class_obj_tab});
}

const std::string RuntimeMips::SYMBOLS[RuntimeMipsSymbolsSize] = {
    "equality_test", "Object.copy",         "_case_abort",       "_case_abort2", "_dispatch_abort",

    "_GenGC_Assign", "_MemMgr_INITIALIZER", "_MemMgr_COLLECTOR", "_GenGC_Init",  "_MemMgr_TEST",    "_GenGC_Collect",

    "_int_tag",      "_bool_tag",           "_string_tag",

    "heap_start",

    "class_objTab",  "class_nameTab"};