#include "TzHeap.h"
#include "TzKern.h"
#include "Paging.h"

// extern'd to Paging.c!
TzMemoryHeap *kernel_heap = NULL;

static Bool HeadLessThanFunc(OrderedMapItem a, OrderedMapItem b) {
    return (((TzHeapBlockHead *)a)->block_size < ((TzHeapBlockHead *)b)->block_size) ? 1 : 0;
}

static Int32 FindSmallestHole(TzMemoryHeap *heap, UInt32 size, Bool page_aligned) {
    UInt32 iterator = 0;
    while (iterator < heap->index.size) {
        TzHeapBlockHead *head = (TzHeapBlockHead *)OrderedArrayGet(&heap->index, iterator);

        if (page_aligned) {
            UInt32 location = (UInt32)head;
            Int32 offset = 0;
            if (((location + sizeof(TzHeapBlockHead)) & 0xFFFFF000) != 0)
                offset = 0x1000 - (location + sizeof(TzHeapBlockHead) % 0x1000);

            Int32 hole_size = (Int32)head->block_size - offset;

            if (hole_size >= (Int32)size)
                break;
        }
        else {
            if (head->block_size >= size) {
                break;
            }
        }
        iterator++;
    }

    // could not find anything, return none
    if (iterator >= heap->index.size) {
        return -1;
    }

    return iterator;
}

TzMemoryHeap *TzMemoryHeapCreate(UInt32 start, UInt32 end, UInt32 max, UChar heap_page_flags) {
    TzMemoryHeap *heap = (TzMemoryHeap *)TzKernAlloc(sizeof(TzMemoryHeap));

    TzAssert((start % 0x1000) == 0);
    TzAssert((end % 0x1000) == 0);


    OrderedArray array = OrderedArrayPlace((void *)start, TZ_HEAP_INDEX_SIZE, &HeadLessThanFunc);
//    heap->index = array;
    TzMemoryCopy(&heap->index, &array, sizeof(OrderedArray));

    start += sizeof(OrderedMapItem) * TZ_HEAP_INDEX_SIZE;

    if ((start & 0xFFFFF000) != 0) {
        start &= 0xFFFFF000;
        start += 0x1000;
    }

    heap->start_addr = start;
    heap->end_addr = end;
    heap->top_addr = max;
    heap->page_flags = heap_page_flags;

    TzHeapBlockHead *hole = (TzHeapBlockHead *)start;
    hole->block_size = end - start;
    hole->magic = TZ_HEAP_MAGIC;
    hole->is_hole = true;

    OrderedArrayAdd(&heap->index, (void *)hole);

    return heap;
}


static void HeapExpand(TzMemoryHeap *heap, UInt32 new_size) {
    TzAssert(new_size > (heap->end_addr - heap->start_addr));
    if ((new_size & 0xFFFFF000) != 0) {
        new_size &= 0xFFFFF000;
        new_size += 0x1000;
    }

    TzAssert((heap->start_addr + new_size) <= heap->top_addr);

    UInt32 old_size = heap->end_addr - heap->start_addr;
    UInt32 index = old_size;
    while (index < new_size) {
        TzAllocFrame(
            TzPageGet(heap->start_addr + index, 1, TzPagingGetKernelDirectory()),
            (Bool)(heap->page_flags & TZ_HEAP_PAGE_FLAGS_SUPERVISOR),
            (Bool)(heap->page_flags & TZ_HEAP_PAGE_FLAGS_READONLY)
        );
        index += 0x1000;
    }
    heap->end_addr = heap->start_addr + new_size;
}

static UInt32 HeapContract(TzMemoryHeap *heap, UInt32 new_size) {
    TzAssert(new_size < (heap->end_addr - heap->start_addr));
    if (new_size & 0x1000) {
        new_size &= 0x1000;
        new_size += 0x1000;
    }

    if (new_size < TZ_HEAP_MIN_SIZE)
        new_size = TZ_HEAP_MIN_SIZE;

    UInt32 old_size = heap->end_addr - heap->start_addr;
    UInt32 index = old_size - 0x1000;

    while (new_size < index) {
        TzFreeFrame(TzPageGet(heap->start_addr + index, 0, TzPagingGetKernelDirectory()));
        index -= 0x1000;
    }

    heap->end_addr = heap->start_addr + new_size;

    return new_size;
}


void *TzMemoryHeapAlloc(TzMemoryHeap *heap, UInt32 size, Bool page_align)
{

    // Make sure we take the size of header/footer into account.
    UInt32 new_size = size + sizeof(TzHeapBlockHead) + sizeof(TzHeapBlockTail);
    // Find the smallest hole that will fit.
    Int32 iterator = FindSmallestHole(heap, new_size, page_align);

    if (iterator == -1) // If we didn't find a suitable hole
    {
        // Save some previous data.
        UInt32 old_length = heap->end_addr - heap->start_addr;
        UInt32 old_end_address = heap->end_addr;

        // We need to allocate some more space.
        HeapExpand(heap, old_length + new_size);
        UInt32 new_length = heap->end_addr - heap->start_addr;

        // Find the endmost header. (Not endmost in size, but in location).
        iterator = 0;
        // Vars to hold the index of, and value of, the endmost header found so far.
        UInt32 idx = -1;
        UInt32 value = 0;

        while (iterator < heap->index.size)
        {
            UInt32 tmp = (UInt32)OrderedArrayGet(&heap->index, iterator);
            if (tmp > value)
            {
                value = tmp;
                idx = iterator;
            }
            iterator++;
        }

        // If we didn't find ANY headers, we need to add one.
        if (idx == -1) {
            TzHeapBlockHead *head = (TzHeapBlockHead *)old_end_address;
            head->magic = TZ_HEAP_MAGIC;
            head->block_size = new_length - old_length;
            head->is_hole = true;

            TzHeapBlockTail *tail = (TzHeapBlockTail *) (old_end_address + head->block_size - sizeof(TzHeapBlockTail));
            tail->magic = TZ_HEAP_MAGIC;
            tail->head = head;
            OrderedArrayAdd(&heap->index, (void*)head);
        }
        else {
            // The last header needs adjusting.
            TzHeapBlockHead *head = OrderedArrayGet(&heap->index, idx);
            head->block_size += new_length - old_length;
            // Rewrite the footer.
            TzHeapBlockTail *tail = (TzHeapBlockTail *)((UInt32)head + head->block_size - sizeof(TzHeapBlockTail));
            tail->head = head;
            tail->magic = TZ_HEAP_MAGIC;
        }
        // We now have enough space. Recurse, and call the function again.
        return TzMemoryHeapAlloc(heap, size, page_align);
    }

    TzHeapBlockHead *orig_hole_header = (TzHeapBlockHead *)OrderedArrayGet(&heap->index, iterator);
    UInt32 orig_hole_pos = (UInt32)orig_hole_header;
    UInt32 orig_hole_size = orig_hole_header->block_size;
    // Here we work out if we should split the hole we found into two parts.
    // Is the original hole size - requested hole size less than the overhead for adding a new hole?
    if (orig_hole_size-new_size < sizeof(TzHeapBlockHead) + sizeof(TzHeapBlockTail))
    {
        // Then just increase the requested size to the size of the hole we found.
        size += orig_hole_size-new_size;
        new_size = orig_hole_size;
    }

    // If we need to page-align the data, do it now and make a new hole in front of our block.
    if (page_align && orig_hole_pos&0xFFFFF000)
    {
        UInt32 new_location   = orig_hole_pos + 0x1000 /* page size */ - (orig_hole_pos&0xFFF) - sizeof(TzHeapBlockHead);
        TzHeapBlockHead *hole_header = (TzHeapBlockHead *)orig_hole_pos;
        hole_header->block_size     = 0x1000 /* page size */ - (orig_hole_pos&0xFFF) - sizeof(TzHeapBlockHead);
        hole_header->magic    = TZ_HEAP_MAGIC;
        hole_header->is_hole  = true;
        TzHeapBlockTail *hole_footer = (TzHeapBlockTail *) ( (UInt32)new_location - sizeof(TzHeapBlockTail) );
        hole_footer->magic    = TZ_HEAP_MAGIC;
        hole_footer->head   = hole_header;
        orig_hole_pos         = new_location;
        orig_hole_size        = orig_hole_size - hole_header->block_size;
    }
    else
    {
        // Else we don't need this hole any more, delete it from the index.
        OrderedArrayRemove(&heap->index, iterator);
    }

    // Overwrite the original header...
    TzHeapBlockHead *block_header  = (TzHeapBlockHead *)orig_hole_pos;
    block_header->magic     = TZ_HEAP_MAGIC;
    block_header->is_hole   = 0;
    block_header->block_size      = new_size;
    // ...And the footer
    TzHeapBlockTail *block_footer  = (TzHeapBlockTail *) (orig_hole_pos + sizeof(TzHeapBlockHead) + size);
    block_footer->magic     = TZ_HEAP_MAGIC;
    block_footer->head    = block_header;

    // We may need to write a new hole after the allocated block.
    // We do this only if the new hole would have positive size...
    if (orig_hole_size - new_size > 0)
    {
        TzHeapBlockHead *hole_header = (TzHeapBlockHead *) (orig_hole_pos + sizeof(TzHeapBlockHead) + size + sizeof(TzHeapBlockTail));
        hole_header->magic    = TZ_HEAP_MAGIC;
        hole_header->is_hole  = 1;
        hole_header->block_size     = orig_hole_size - new_size;
        TzHeapBlockTail *hole_footer = (TzHeapBlockTail *) ( (UInt32)hole_header + orig_hole_size - new_size - sizeof(TzHeapBlockTail) );
        if ((UInt32)hole_footer < heap->end_addr)
        {
            hole_footer->magic = TZ_HEAP_MAGIC;
            hole_footer->head = hole_header;
        }
        // Put the new hole in the index;
        OrderedArrayAdd(&heap->index, (void*)hole_header);
    }

    return (void *) ( (UInt32)block_header+sizeof(TzHeapBlockHead) );
}



void TzMemoryHeapFree(TzMemoryHeap *heap, void *ptr)
{
    if (!ptr)
        return;

    // Get the header and footer associated with this pointer.
    TzHeapBlockHead *header = (TzHeapBlockHead * )((UInt32) ptr - sizeof(TzHeapBlockHead));
    TzHeapBlockTail *footer = (TzHeapBlockTail * )((UInt32) header + header->block_size - sizeof(TzHeapBlockTail));

    // Sanity checks.
    TzAssert(header->magic == TZ_HEAP_MAGIC);
    TzAssert(footer->magic == TZ_HEAP_MAGIC);

    // Make us a hole.
    header->is_hole = true;

    // Do we want to add this header into the 'free holes' index?
    char do_add = 1;

    // Unify left
    // If the thing immediately to the left of us is a footer...
    TzHeapBlockTail *test_footer = (TzHeapBlockTail *) ( (UInt32)header - sizeof(TzHeapBlockTail) );
    if (test_footer->magic == TZ_HEAP_MAGIC && test_footer->head->is_hole) {
        UInt32 cache_size = header->block_size; // Cache our current size.
        header = test_footer->head;     // Rewrite our header with the new one.
        footer->head = header;          // Rewrite our footer to point to the new header.
        header->block_size += cache_size;       // Change the size.
        do_add = 0;                       // Since this header is already in the index, we don't want to add it again.
    }

    // Unify right
    // If the thing immediately to the right of us is a header...
    TzHeapBlockHead *test_header = (TzHeapBlockHead *)((UInt32)footer + sizeof(TzHeapBlockTail));
    if (test_header->magic == TZ_HEAP_MAGIC && test_header->is_hole) {
        header->block_size += test_header-> block_size; // Increase our size.
        test_footer = (TzHeapBlockTail *) ((UInt32)test_header + // Rewrite it's footer to point to our header.
                                    test_header->block_size - sizeof(TzHeapBlockTail) );
        footer = test_footer;
        // Find and remove this header from the index.
        UInt32 iterator = 0;
        while ( (iterator < heap->index.size) &&
                (OrderedArrayGet(&heap->index, iterator) != (void*)test_header) )
            iterator++;

        // Make sure we actually found the item.
        TzAssert(iterator < heap->index.size);
        // Remove it.
        OrderedArrayRemove(&heap->index, iterator);
    }

    // If the footer location is the end address, we can contract.
    if ( (UInt32)footer+sizeof(TzHeapBlockTail) == heap->end_addr)
    {
        UInt32 old_length = heap->end_addr-heap->start_addr;
        UInt32 new_length = HeapContract(heap, (UInt32)header - heap->start_addr);
        // Check how big we will be after resizing.
        if (header->block_size - (old_length-new_length) > 0)
        {
            // We will still exist, so resize us.
            header->block_size -= old_length-new_length;
            footer = (TzHeapBlockTail *) ( (UInt32)header + header->block_size - sizeof(TzHeapBlockTail) );
            footer->magic = TZ_HEAP_MAGIC;
            footer->head = header;
        }
        else
        {
            // We will no longer exist :(. Remove us from the index.
            UInt32 iterator = 0;
            while ( (iterator < heap->index.size) &&
                    (OrderedArrayGet(&heap->index, iterator) != (void*)test_header) )
                iterator++;
            // If we didn't find ourselves, we have nothing to remove.
            if (iterator < heap->index.size)
                OrderedArrayRemove(&heap->index, iterator);
        }
    }

    if (do_add == 1)
        OrderedArrayAdd(&heap->index, (void*) header);
}