#ifndef TOPAZ_MEMORY_PAGING_H
#define TOPAZ_MEMORY_PAGING_H

#define TZ_PAGE_TABLES_COUNT 1024

#include <Types.h>

typedef struct {
    UInt32 present  : 1;
    UInt32 rw       : 1;
    UInt32 user     : 1;
    UInt32 accessed : 1;
    UInt32 dirty    : 1;
    UInt32 unused   : 7;
    UInt32 frame    : 20;
} TzMemoryPage;

typedef struct {
    TzMemoryPage pages[1024];
} TzPageTable;


typedef struct {
    TzPageTable *tables[TZ_PAGE_TABLES_COUNT];
    UInt32 tables_physical[TZ_PAGE_TABLES_COUNT];
    UInt32 physical_addr;
} TzPageDirectory;

UInt32 TzKernInternAlloc(UInt32 size, Bool align, UInt32 *phys);
UInt32 TzKernAlloc(UInt32 size);
UInt32 TzKernAllocAlign(UInt32 size);
void TzKernFree(void *ptr);

void TzPagingInit();
void TzPageSwitchDir(TzPageDirectory *new_dir);
TzMemoryPage *TzPageGet(UInt32 addr, Int make, TzPageDirectory *dir);

void TzFrameSet(UInt32 addr);
void TzFrameClear(UInt32 addr);
void TzAllocFrame(TzMemoryPage *page, Bool is_kpage, Bool writeable);
void TzFreeFrame(TzMemoryPage *page);

TzPageDirectory *TzPagingGetKernelDirectory();
TzPageDirectory  *TzPagingGetCurrentDirectory();

#endif