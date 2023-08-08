#include <Types.h>
#include <ArchX86.h>
#include <TzKern.h>

#include <Driver/Interrupt.h>
#include <Driver/TtyGraphics.h>

#include <TzString.h>
#include <TzKern.h>

#include <Mem/Paging.h>

#define TZ_INDEX_FROM_BIT(a) (a / (8 * 4))
#define TZ_OFFSET_FROM_BIT(a) (a % (8 * 4))

#define TZ_GET_FRAME_N(addr) (addr / 0x1000)

static UInt32 frames_count;
static UInt32 *frames;

extern UInt32 end;
UInt32 placement_address = (UInt32)&end;

static TzPageDirectory *kernel_directory = NULL;
static TzPageDirectory *current_directory = NULL;

void TzFrameSet(UInt32 addr) {
    UInt32 frame = TZ_GET_FRAME_N(addr);
    UInt32 index = TZ_INDEX_FROM_BIT(frame);
    UInt32 offset = TZ_OFFSET_FROM_BIT(frame);

    frames[index] |= (0x1 << offset);
}

void TzFrameClear(UInt32 addr) {
    UInt32 frame = TZ_GET_FRAME_N(addr);
    UInt32 index = TZ_INDEX_FROM_BIT(frame);
    UInt32 offset = TZ_OFFSET_FROM_BIT(frame);

    frames[index] &= ~(0x1 << offset);
}

UInt32 TzFrameTest(UInt32 addr) {
    UInt32 frame = TZ_GET_FRAME_N(addr);
    UInt32 index = TZ_INDEX_FROM_BIT(frame);
    UInt32 offset = TZ_OFFSET_FROM_BIT(frame);

    return (frames[index] & (0x1 << offset));
}

UInt32 TzFindFirstFreeFrame() {
    UInt32 i;
    UInt32 j;

    for (i = 0; i < TZ_INDEX_FROM_BIT(frames_count); i++) {
        // nothing free, exit
        if (frames[i] == 0xFFFFFFFF) {
            continue;
        }
        for (j = 0; j < 32; j++) {
            UInt32 test = 0x1 << j;
            if (!(frames[i] & test)) {
                return i * 4 * 8 + j;
            }
        }
    }
    return 0;
}

void TzAllocFrame(TzMemoryPage *page, Bool is_kpage, Bool writeable) {
    if (page->frame) {
        return;
    }

    UInt32 index = TzFindFirstFreeFrame();
    if (index == (UInt32) - 1) {
        TzPanic("No free frames!", 0);
    }
    TzFrameSet(index * 0x1000);
    page->present = 1;
    page->rw = (writeable) ? 1 : 0;
    page->user = (is_kpage) ? 0 : 1;
    page->frame = index;
}

void TzFreeFrame(TzMemoryPage *page) {
    UInt32 frame;
    if (!(frame = page->frame))
        return;
    TzFrameClear(frame);
    page->frame = 0x00;
}

static void PageFaultHandler(TzRegisterList *registers) {
    UInt32 faulting_address;
    __asm__ volatile ("mov %%cr2, %0" : "=r" (faulting_address));

    // The error code gives us details of what happened.
    int present   = !(registers->err_code & 0x1); // Page not present
    int rw = registers->err_code & 0x2;           // Write operation?
    int us = registers->err_code & 0x4;           // Processor was in user-mode?
    int reserved = registers->err_code & 0x8;     // Overwritten CPU-reserved bits of page entry?
    int id = registers->err_code & 0x10;          // Caused by an instruction fetch?

    TzPanic(
        "PAGE FAULT ( %s%s%s%s)%s AT 0x%x\0",
        (present ? "PRESENT " : ""),
        (rw ? "READONLY " : ""),
        (us ? "USERMODE " : ""),
        (reserved ? "RESERVED " : ""),
        (id ? " << INSTR. FETCH >>" : ""),
        faulting_address
    );

    TzHalt();
}

UInt32 TzKernInternAlloc(UInt32 size, Bool align, UInt32 *phys) {
    if (align == 1 && (placement_address & 0xFFFFF000)) {
        placement_address &= 0xFFFFF000;
        placement_address += 0x1000;
    }

    if (phys) {
        *phys = placement_address;
    }

    UInt32 temp = placement_address;
    placement_address += size;

    return temp;
}

UInt32 TzKernAlloc(UInt32 size) {
    return TzKernInternAlloc(size, 0, NULL);
}

UInt32 TzKernAllocAlign(UInt32 size) {
    return TzKernInternAlloc(size, 1, NULL);
}

void TzPagingInit() {
    // 16MB
    UInt32 end_page = 0x1000000;

    frames_count = end_page / 0x1000;
    frames = (UInt32 *)TzKernAlloc(TZ_INDEX_FROM_BIT(frames_count));
    TzMemorySet(frames, 0, TZ_INDEX_FROM_BIT(frames_count));

    kernel_directory = (TzPageDirectory *) TzKernAllocAlign(sizeof(TzPageDirectory));
    TzMemorySet(kernel_directory, 0, sizeof(TzPageDirectory));
    current_directory = kernel_directory;

    Int i = 0;
    while (i < placement_address) {
        TzAllocFrame(TzPageGet(i, 1, kernel_directory), 0, 0);
        i += 0x1000;
    }

    TzRegisterIrq(14, PageFaultHandler);
    TzPageSwitchDir(kernel_directory);
}

void TzPageSwitchDir(TzPageDirectory *new_dir) {
    current_directory = new_dir;
    __asm__ volatile ("mov %0, %%cr3":: "r"(&new_dir->tables_physical));
    UInt32 cr0;
    __asm__ volatile ("mov %%cr0, %0": "=r"(cr0));
    cr0 |= 0x80000000;
    __asm__ volatile ("mov %0, %%cr0":: "r"(cr0));
}

TzMemoryPage *TzPageGet(UInt32 addr, Int make, TzPageDirectory *dir) {
    // get our page index
    addr /= 0x1000;

    // find the page table containing this address
    UInt32 table_index = addr / 1024;

    // if this table is already assigned
    if (dir->tables[table_index]) {
        return &dir->tables[table_index]->pages[addr % 1024];
    }
    else if (make) {
        UInt32 temp = 0;
        dir->tables[table_index] = (TzPageTable *)TzKernInternAlloc(sizeof(TzPageTable), 1, &temp);
        TzMemorySet(dir->tables[table_index], 0, 0x1000);
        dir->tables_physical[table_index] = temp | 0x7;
        return &dir->tables[table_index]->pages[addr % 1024];
    }

    return 0;
}
