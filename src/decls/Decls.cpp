#include "Decls.h"

const char *const ObjectMethodsNames[ObjectMethodsSize] = {"abort", "type_name", "copy"};

const char *const IntMethodsNames[ObjectMethodsSize] = {"abort", "type_name", "copy"};

const char *const IOMethodsNames[IOMethodsSize] = {"abort",   "type_name", "copy",  "out_string",
                                                   "out_int", "in_string", "in_int"};

const char *const StringMethodsNames[StringMethodsSize] = {"abort", "type_name", "copy", "length", "concat", "substr"};

const char *const BaseClassesNames[BaseClassesSize] = {"Object", "Int", "Bool", "String", "IO", "SELF_TYPE"};

const char *MainMethodName = "main";

const char *MainClassName = "Main";

const char *SelfObject = "self";
