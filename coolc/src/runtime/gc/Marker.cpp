#include "Marker.hpp"
#include "runtime/gc/Allocator.hpp"

using namespace gc;

Marker *Marker::MarkerObj = nullptr;

Marker::Marker(address heap_start, address heap_end) : _heap_start(heap_start), _heap_end(heap_end)
{
}

Marker::~Marker()
{
}

void Marker::init(GcType type)
{
    Allocator *alloca = Allocator::allocator();

    switch (type)
    {
    case ZEROGC:
        break;
#if defined(LLVM_SHADOW_STACK) || defined(LLVM_STATEPOINT_EXAMPLE)
    case MARKSWEEPGC:
    case THREADED_MC_GC:
        MarkerObj = new MarkerFIFO(alloca->start(), alloca->end());
        break;
    case COMPRESSOR_GC:
        MarkerObj = new BitMapMarker(alloca->start(), alloca->end());
        break;
#endif // LLVM_SHADOW_STACK || LLVM_STATEPOINT_EXAMPLE
    default:
        fprintf(stderr, "cannot select GC!\n");
        abort();
        break;
    };
}

void Marker::release()
{
    delete MarkerObj;
    MarkerObj = nullptr;
}

void MarkerFIFO::mark_root(void *obj, address *root, const address *meta)
{
    MarkerFIFO *mrkr = (MarkerFIFO *)obj;
    mrkr->mark_root(root);
}

void MarkerFIFO::mark_from_roots()
{
    assert(_worklist.empty());
    StackWalker::walker()->process_roots(this, &MarkerFIFO::mark_root, true);
}

bool MarkerFIFO::is_marked(ObjectLayout *object) const
{
    return object->is_marked();
}

void MarkerFIFO::mark_unmarked_object(ObjectLayout *object)
{
    object->set_marked();
}

void MarkerFIFO::mark_root(address *root)
{
    ObjectLayout *obj = (ObjectLayout *)(*root);

#ifdef DEBUG
    if (TraceMarking)
    {
        fprintf(stderr, "Try mark: %p of size %lu in %p\n", obj, !obj ? 0 : obj->_size, root);
    }
#endif // DEBUG

    if (obj && !is_marked(obj))
    {
        mark_unmarked_object(obj);
        _worklist.push(obj);
        mark();
    }
}

void MarkerFIFO::mark()
{
    while (!_worklist.empty())
    {
        ObjectLayout *hdr = _worklist.front();
        _worklist.pop();

        assert(hdr);
        if (hdr->has_special_type())
        {
            if (hdr->is_string())
            {
                StringLayout *str = (StringLayout *)hdr;
                IntLayout *size = str->_string_size;
                if (size && !is_marked(size))
                {
                    _worklist.push(size);
                    mark_unmarked_object(size);
                }
            }
            continue;
        }

        int fields_cnt = hdr->field_cnt();
        address *fields = hdr->fields_base();

        for (int j = 0; j < fields_cnt; j++)
        {
            ObjectLayout *child = (ObjectLayout *)fields[j];
            if (child && !is_marked(child))
            {
                mark_unmarked_object(child);
                _worklist.push(child);
            }
        }
    }
}

BitMapMarker::BitMapMarker(address heap_start, address heap_end) : MarkerFIFO(heap_start, heap_end)
{
    int long_words = bits_num() / BITS_PER_BIT_MAP_WORD;
    _bitmap.resize(long_words);
}

void BitMapMarker::mark_root(void *obj, address *root, const address *meta)
{
    BitMapMarker *mrkr = (BitMapMarker *)obj;
    mrkr->mark_root(root);
}

void BitMapMarker::mark_from_roots()
{
    assert(_worklist.empty());
    StackWalker::walker()->process_roots(this, &BitMapMarker::mark_root, true);
}

void BitMapMarker::mark_unmarked_object(ObjectLayout *object)
{
    assert(is_alligned((address)object - _heap_start));
    size_t bit_num = (size_t)((address)object - _heap_start) / BYTES_PER_BIT;

    size_t first_word_num = bit_num / BITS_PER_BIT_MAP_WORD;
    assert(first_word_num < _bitmap.size());

    size_t mask1 = ~((1llu << (bit_num % BITS_PER_BIT_MAP_WORD)) - 1);

    assert(is_alligned(object->_size));
    size_t object_size_bits = object->_size / BYTES_PER_BIT - 1;
    size_t last_word_num = (bit_num + object_size_bits) / BITS_PER_BIT_MAP_WORD;
    assert(last_word_num < _bitmap.size());

    size_t mask2 = 0;

    int shift = ((bit_num + object_size_bits) % BITS_PER_BIT_MAP_WORD + 1);
    if (shift == BITS_PER_BIT_MAP_WORD)
    {
        mask2 = ~0llu;
    }
    else
    {
        mask2 = ((1llu << shift) - 1);
    }

    if (first_word_num == last_word_num)
    {
        size_t final_mask = mask1 & mask2;

        assert((_bitmap[first_word_num] & final_mask) == 0);
        _bitmap[first_word_num] |= final_mask;
    }
    else
    {
        assert((_bitmap[first_word_num] & mask1) == 0);
        _bitmap[first_word_num] |= mask1;

        assert((_bitmap[last_word_num] & mask2) == 0);
        _bitmap[last_word_num] |= mask2;

        for (int i = first_word_num + 1; i < last_word_num; i++)
        {
            _bitmap[i] |= ~0llu;
        }
    }

#ifdef DEBUG
    for (size_t b = byte_to_bit((address)object); b < byte_to_bit((address)object + object->_size); b++)
    {
        if (!is_bit_set(b))
        {
            fprintf(stderr, "Bit %zu is not set in range [%zu-%zu)!\n", b, byte_to_bit((address)object),
                    byte_to_bit((address)object + object->_size));
        }
        assert(is_bit_set(b));
    }
#endif // DEBUG
}

bool BitMapMarker::is_marked(ObjectLayout *object) const
{
    if (!Allocator::allocator()->is_heap_addr((address)object))
    {
        // constants are always marked
        assert(object->is_marked());
        return true;
    }

    // it's enough to check the first bit
    return is_bit_set(byte_to_bit((address)object));
}

void BitMapMarker::clear()
{
    std::fill(_bitmap.begin(), _bitmap.end(), 0);
}