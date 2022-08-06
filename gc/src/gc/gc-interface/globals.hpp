#include "gc/gc-impl/alloca.inline.hpp"
#include "gc/gc-impl/gc-common.inline.hpp"
#include "gc/gc-impl/mark-compact/gc-mark-compact.inline.hpp"
#include "gc/gc-impl/mark-sweep/gc-mark-sweep.inline.hpp"
#include "gc/gc-impl/mark.inline.hpp"
#include "gc/gc-impl/object.inline.hpp"

#define IIS_HEADER objects::ObjectHeader<int, int, size_t>
#define AIS_HEADER objects::ObjectHeader<address, int, size_t>