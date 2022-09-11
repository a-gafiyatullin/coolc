#pragma once

#include "codegen/runtime/globals.hpp"
#include <cstdint>
#include <unordered_map>
#include <vector>

extern address __LLVM_StackMaps;            // NOLINT
extern thread_local address _stack_pointer; // NOLINT

namespace gc
{
namespace stackmap
{

// https://llvm.org/docs/StackMaps.html#stack-map-format
// https://llvm.org/docs/Statepoints.html#stack-map-format

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

enum DWARFRegNum
{
    SP = 0x7
};

enum LocationType
{
    Register = 0x1,
    Direct,
    Indirect,
    Constant,
    ConstIndex
};

// represents info about relocation
// if _base_offset != _offset this is a derived pointer
struct LocInfo
{
    int _base_offset;
    int _offset;
};

// represent info about all relocations at safepoint
struct AddrInfo
{
    int _stack_size;               // offset to find previous activation
    std::vector<LocInfo> _offsets; // offsets relative to sp
};

class StackMap
{
  private:
    static StackMap *Map;

    std::unordered_map<address, AddrInfo> _stack_maps;

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

    /**
     * @brief Get info about stack by addr
     *
     * @param ret Return value
     * @return AddrInfo&
     */
    const AddrInfo *info(address ret) const;
};

}; // namespace stackmap
}; // namespace gc