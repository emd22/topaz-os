#ifndef TOPAZ_OS_MEMORY_HEAP_H
#define TOPAZ_OS_MEMORY_HEAP_H

#include "OrderedMap.h"
#include "Types.h"

#define TZ_KHEAP_START        0xC0000000
#define TZ_KHEAP_INITIAL_SIZE 0x100000

#define TZ_HEAP_INDEX_SIZE    0x20000
#define TZ_HEAP_MAGIC         0xA3A2A1A0
#define TZ_HEAP_MIN_SIZE      0x70000


typedef enum {
    TZ_HEAP_PAGE_FLAGS_NONE       = 0x0000,
    TZ_HEAP_PAGE_FLAGS_SUPERVISOR = 0x0001,
    TZ_HEAP_PAGE_FLAGS_READONLY   = 0x0002,
} TzHeapPageFlags;


typedef struct {
    UInt32 magic;
    Bool is_hole;
    UInt32 block_size;
} TzHeapBlockHead;

typedef struct {
    UInt32 magic;
    TzHeapBlockHead *head;
} TzHeapBlockTail;

typedef struct {
    OrderedArray index;
    UInt32 start_addr;
    UInt32 end_addr;
    UInt32 top_addr;
    TzHeapPageFlags page_flags;
} TzMemoryHeap;


TzMemoryHeap *TzMemoryHeapCreate(UInt32 start, UInt32 end, UInt32 max, UChar heap_page_flags);
void *TzMemoryHeapAlloc(TzMemoryHeap *heap, UInt32 size, Bool page_aligned);
void  TzMemoryHeapFree(TzMemoryHeap *heap, void *ptr);

#endif