#include "StackMap.hpp"
#include <cstdio>

using namespace gc::stackmap;

StackMap *StackMap::Map = nullptr;

StackMap::StackMap()
{
    Header *hdr = (Header *)&__LLVM_StackMaps;
    if (PrintStackMaps)
    {
        fprintf(stderr, "StackMaps:\n");
        fprintf(stderr, "Version     : %d\n", hdr->_version);
        fprintf(stderr, "Reserved    : %d\n", hdr->_reserved0);
        fprintf(stderr, "Reserved    : %d\n", hdr->_reserved1);
        fprintf(stderr, "NumFunctions: %d\n", hdr->_num_functions);
        fprintf(stderr, "NumConstants: %d\n", hdr->_num_constants);
        fprintf(stderr, "NumRecords  : %d\n", hdr->_num_records);
        fprintf(stderr, "----------------\n\n");
    }

    address start = (address)&__LLVM_StackMaps + sizeof(Header);

    for (int i = 0; i < hdr->_num_functions; i++)
    {
        StkSizeRecord *rec = (StkSizeRecord *)start;

        if (PrintStackMaps)
        {
            fprintf(stderr, "StkSizeRecord %d:\n", i);
            fprintf(stderr, "Function Address : %p\n", rec->_func_address);
            fprintf(stderr, "Stack Size       : %lu\n", rec->_stack_size);
            fprintf(stderr, "Record Count     : %lu\n\n", rec->_record_count);
        }

        start += sizeof(StkSizeRecord);
    }

    if (PrintStackMaps)
    {
        fprintf(stderr, "----------------------------\n\n");
    }

    for (int i = 0; i < hdr->_num_constants; i++)
    {
        Constant *con = (Constant *)start;

        if (PrintStackMaps)
        {
            fprintf(stderr, "Constant %d:\n", i);
            fprintf(stderr, "LargeConstant    : %lu\n", con->_large_constant);
        }

        start += sizeof(Constant);
    }

    if (PrintStackMaps)
    {
        fprintf(stderr, "----------------------------\n\n");
    }

    for (int i = 0; i < hdr->_num_records; i++)
    {
        StkMapRecord *stkmap = (StkMapRecord *)start;

        if (PrintStackMaps)
        {
            fprintf(stderr, "StkMapRecord %d:\n", i);
            fprintf(stderr, "PatchPoint ID     : %lu\n", stkmap->_patch_point_id);
            fprintf(stderr, "Instruction Offset: %d\n", stkmap->_instruction_offset);
            fprintf(stderr, "Reserved          : %d\n", stkmap->_reserved);
            fprintf(stderr, "NumLocations      : %d\n\n", stkmap->_num_locations);
        }

        start += sizeof(StkMapRecord);

        for (int i = 0; i < stkmap->_num_locations; i++)
        {
            Location *lock = (Location *)start;

            if (PrintStackMaps)
            {
                fprintf(stderr, "   Location %d:\n", i);
                fprintf(stderr, "   Type                   : %d\n", lock->_type);
                fprintf(stderr, "   Reserved               : %d\n", lock->_reserved0);
                fprintf(stderr, "   Location Size          : %d\n", lock->_location_size);
                fprintf(stderr, "   Dwarf RegNum           : %d\n", lock->_dwarf_reg_num);
                fprintf(stderr, "   Reserved               : %d\n", lock->_reserved1);
                fprintf(stderr, "   Offset or SmallConstant: %d\n\n", lock->_offset_or_small_constant);
            }

            start += sizeof(Location);
        }

        if (((uint64_t)start & 0x7) != 0)
        {
            if (PrintStackMaps)
            {
                fprintf(stderr, "Padding    : %d\n", *((uint32_t *)start));
            }

            start += sizeof(uint32_t);
        }

        StkMapRecordTail *tail = (StkMapRecordTail *)(start);

        if (PrintStackMaps)
        {
            fprintf(stderr, "Padding    : %d\n", tail->_padding);
            fprintf(stderr, "NumLiveOuts: %d\n\n", tail->_num_live_outs);
        }

        start += sizeof(StkMapRecordTail);

        for (int i = 0; i < tail->_num_live_outs; i++)
        {
            LiveOuts *live = (LiveOuts *)start;

            if (PrintStackMaps)
            {
                fprintf(stderr, "   LiveOut %d:\n", i);
                fprintf(stderr, "   Dwarf RegNum : %d\n", live->_dwarf_reg_num);
                fprintf(stderr, "   Reserved     : %d\n", live->_reserved);
                fprintf(stderr, "   Size in Bytes: %d\n\n", live->_sizein_bytes);
            }

            start += sizeof(LiveOuts);
        }

        if (((uint64_t)start & 0x7) != 0)
        {
            if (PrintStackMaps)
            {
                fprintf(stderr, "Padding    : %d\n", *((uint32_t *)start));
            }

            start += sizeof(uint32_t);
        }
    }
}

void StackMap::init()
{
    Map = new StackMap;
}

void StackMap::release()
{
    delete Map;
}