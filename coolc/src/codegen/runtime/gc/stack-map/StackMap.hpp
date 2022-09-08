#pragma once

#include "codegen/runtime/globals.hpp"
#include <cstdint>

extern address __LLVM_StackMaps; // NOLINT

namespace gc
{
namespace stackmap
{

struct Header
{
    uint8_t _version;
    uint8_t _reserved0;
    uint16_t _reserved1;

    uint32_t _num_functions;
    uint32_t _num_constants;
    uint32_t _num_records;
};

struct StkSizeRecord
{
    address _func_address;
    uint64_t _stack_size;
    uint64_t _record_count;
};

struct Constant
{
    uint64_t _large_constant;
};

struct StkMapRecord
{
    uint64_t _patch_point_id;
    uint32_t _instruction_offset;
    uint16_t _reserved;
    uint16_t _num_locations;
};

struct Location
{
    uint8_t _type;
    uint8_t _reserved0;
    uint16_t _location_size;
    uint16_t _dwarf_reg_num;
    uint16_t _reserved1;
    int32_t _offset_or_small_constant;
};

struct LiveOuts
{
    uint16_t _dwarf_reg_num;
    uint8_t _reserved;
    uint8_t _sizein_bytes;
};

struct StkMapRecordTail
{
    uint16_t _padding;
    uint16_t _num_live_outs;
};

class StackMap
{
  private:
    static StackMap *Map;

  public:
    StackMap();

    /**
     * @brief Parse __LLVM_StackMaps section
     *
     */
    static void init();

    /**
     * @brief Deallocate Stack Map
     *
     */
    static void release();

    /**
     * @brief Get a global StackMap
     *
     * @return A global StackMap
     */
    static StackMap *map()
    {
        return Map;
    }
};

}; // namespace stackmap
}; // namespace gc