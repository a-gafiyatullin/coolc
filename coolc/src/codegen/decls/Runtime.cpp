#include "Runtime.h"

using namespace codegen;

const char *const codegen::RuntimeMethodsNames[RuntimeMethodsSize] = {"equals", "gc_alloc", "gc_alloc_by_tag"};

const char *const codegen::RuntimeStructuresNames[RuntimeStructuresSize] = {"ClassNameTab", "ClassObjTab"};