#include "StackMap.hpp"
#include <cassert>
#include <cstdio>

using namespace gc::stackmap;

StackMap *StackMap::Map = nullptr;

StackMap::StackMap()
{
    Header *hdr = (Header *)&__LLVM_StackMaps;

    assert(hdr->_version == 3);
    assert(hdr->_num_constants == 0);
    assert(hdr->_reserved0 == 0);
    assert(hdr->_reserved1 == 0);

    StkSizeRecord *funcs = (StkSizeRecord *)((address)&__LLVM_StackMaps + sizeof(Header));
    address recrds = (address)(funcs + hdr->_num_functions);

    for (int i = 0; i < hdr->_num_functions; i++)
    {
        StkSizeRecord &func = funcs[i];

        assert(func._record_count >= 1);

        std::vector<LocInfo> always_live;
        std::vector<AddrInfo *> delayed;
        bool found_locals = false;

        for (int j = 0; j < func._record_count; j++)
        {
            StkMapRecord *stkmap = (StkMapRecord *)recrds;

            assert(stkmap->_reserved == 0);
            recrds += sizeof(StkMapRecord);

            AddrInfo &info = _stack_maps[func._func_address + stkmap->_instruction_offset];
            info._stack_size = func._stack_size;

            if (found_locals)
            {
                if (info._offsets.empty())
                    info._offsets.insert(info._offsets.end(), always_live.begin(), always_live.end());
            }
            else
                delayed.push_back(&info);

            // a special record that include all locals
            // they are all direct locations with id 0
            if (stkmap->_patch_point_id == 0)
            {
                found_locals = true;
                for (int k = 0; k < stkmap->_num_locations; k++)
                {
                    Location *lock = (Location *)recrds;

                    assert(lock->_reserved1 == 0);
                    assert(lock->_type == LocationType::Direct);
                    assert(lock->_dwarf_reg_num == DWARFRegNum::SP || lock->_dwarf_reg_num == DWARFRegNum::FP);
                    assert(lock->_location_size == 8);

                    LocInfo info;

                    info._base_reg = (DWARFRegNum)lock->_dwarf_reg_num;
                    info._der_reg = (DWARFRegNum)lock->_dwarf_reg_num;

                    info._base_offset = lock->_offset_or_small_constant;
                    info._offset = lock->_offset_or_small_constant;

                    always_live.push_back(info);

                    recrds += sizeof(Location);
                }
            }
            else
            {
                assert(stkmap->_num_locations >= 3); // skip the first three locs

                for (int k = 0; k < 3; k++)
                {
                    Location *lock = (Location *)recrds;

                    assert(lock->_reserved1 == 0);
                    assert(lock->_type == LocationType::Constant);
                    assert(lock->_offset_or_small_constant == 0);

                    recrds += sizeof(Location);
                }

                assert((stkmap->_num_locations - 3) % 2 == 0);
                for (int k = 3; k < stkmap->_num_locations; k += 2)
                {
                    Location *lock1 = (Location *)recrds;
                    Location *lock2 = (Location *)(recrds + sizeof(Location));

                    assert(lock1->_reserved1 == 0);
                    assert(lock1->_type == LocationType::Indirect);
                    assert(lock1->_dwarf_reg_num == DWARFRegNum::SP || lock1->_dwarf_reg_num == DWARFRegNum::FP);
                    assert(lock1->_location_size == 8);

                    assert(lock2->_reserved1 == 0);
                    assert(lock2->_type == LocationType::Indirect);
                    assert(lock2->_dwarf_reg_num == DWARFRegNum::SP || lock2->_dwarf_reg_num == DWARFRegNum::FP);
                    assert(lock2->_location_size == 8);

                    LocInfo info1;

                    info1._base_reg = (DWARFRegNum)lock1->_dwarf_reg_num;
                    info1._der_reg = (DWARFRegNum)lock1->_dwarf_reg_num;

                    info1._base_offset = lock1->_offset_or_small_constant;
                    info1._offset = lock1->_offset_or_small_constant;

                    info._offsets.push_back(info1);

                    if (lock1->_offset_or_small_constant != lock2->_offset_or_small_constant ||
                        lock2->_dwarf_reg_num != lock1->_dwarf_reg_num)
                    {
                        LocInfo info2;

                        info2._base_reg = (DWARFRegNum)lock1->_dwarf_reg_num;
                        info2._der_reg = (DWARFRegNum)lock2->_dwarf_reg_num;

                        info2._base_offset = lock1->_offset_or_small_constant;
                        info2._offset = lock2->_offset_or_small_constant;

                        info._offsets.back() = info2;
                    }

                    // reduce amount of slots
                    if (info._offsets.size() > 1)
                    {
                        auto &prev_info = info._offsets[info._offsets.size() - 2];
                        auto &curr_info = info._offsets.back();
                        if (prev_info._base_reg == curr_info._base_reg &&
                            prev_info._base_offset == curr_info._base_offset)
                        {
                            if (prev_info._base_offset == prev_info._offset &&
                                curr_info._base_offset != curr_info._offset)
                            {
                                // previous info is duplicate
                                prev_info._offset = curr_info._offset;
                                info._offsets.pop_back();
                            }
                            else
                            {
                                // current info is duplicate
                                info._offsets.pop_back();
                            }
                        }
                    }

                    recrds += 2 * sizeof(Location);
                }
            }

            recrds = (address)(((uint64_t)recrds + 0x7) & ~0x7);

            StkMapRecordTail *tail = (StkMapRecordTail *)(recrds);
            assert(tail->_num_live_outs == 0); // skip all LiveOuts
            recrds += sizeof(StkMapRecordTail);

            recrds = (address)(((uint64_t)recrds + 0x7) & ~0x7);
        }

        assert(found_locals);
        for (auto *sfpt : delayed)
        {
            sfpt->_offsets.insert(sfpt->_offsets.end(), always_live.begin(), always_live.end());
        }
    }

#ifdef DEBUG
    if (PrintStackMaps)
    {
        for (const auto &safepoint : _stack_maps)
        {
            fprintf(stderr, "Safepoint address: %p\n", safepoint.first);
            fprintf(stderr, "Stack size: %d\n", safepoint.second._stack_size);
            int i = 0;
            for (const auto &offset : safepoint.second._offsets)
            {
                fprintf(stderr, "%d: Offset(reg = %d) = %d, base offset(reg = %d) = %d\n", i, offset._der_reg,
                        offset._offset, offset._base_reg, offset._base_offset);
                i++;
            }
            fprintf(stderr, "\n");
        }
    }
#endif // DEBUG
}

const AddrInfo *StackMap::info(address ret) const
{
    const auto info = _stack_maps.find(ret);
    if (info != _stack_maps.end())
    {
        return &info->second;
    }

    return nullptr;
}

void StackMap::init()
{
    Map = new StackMap;
}

void StackMap::release()
{
    delete Map;
}