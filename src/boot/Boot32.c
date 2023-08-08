#include <Types.h>

#include <TzKern.h>
#include <ArchX86.h>

#include <Driver/Interrupt.h>
#include <Driver/PitTimer.h>
#include <Driver/TtyGraphics.h>

#include <Mem/Paging.h>

void printc(char ch, int color);
void prints(const char *str, int color);
void kmain(void);

void TzPagingMap(UInt32 virt, UInt32 phys);
void TzPagingInit();

extern void _asm_EnablePaging(UInt32 page_directory);

inline void TzOut8(UInt16 port, UInt8 value);
inline UInt8 TzIn8(UInt16 port);

void TzRegisterIrq(UInt8 int_no, TzIrqHandle handler);

// start of our 32 bit bootloader
extern void _start(void) {
    kmain();

    TzHalt();
}


void TimerCallback(TzRegisterList registers) {
    TzPrintString("z");
}


void kmain(void) {
    TzBackgroundColorSet(TZ_COLOR_CYAN);
    TzForegroundColorSet(TZ_COLOR_YELLOW);
    TzDisplayClear();
    TzSetPosition(0, 0);
    TzPrintString("Welcome to TOPAZ OS!\n");
    TzForegroundColorSet(TZ_COLOR_WHITE);
    TzPrintString("Init: ");

    TzGdtInit();
    TzPrintString("GDT [OK] ");
    TzIdtInit();
    TzPrintString("IDT [OK] ");
    TzPagingInit();
    TzPrintString("MEM32 [OK]\n");

    



    return;
}



/*

typedef struct {
    UInt32 *PagingDir;
    UInt32 DirLocation;
    UInt32 *PagePrev;
} TzMemoryInfo;

static TzMemoryInfo memory_info;



void TzPagingMap(UInt32 virt, UInt32 phys) {
    UInt16 id = virt >> 22;

    int i;
    for (i = 0; i < 1024; i++) {
        memory_info.PagePrev[i] = phys | 3;
        phys += 4096;
    }
    memory_info.PagingDir[id] = ((UInt32)memory_info.PagePrev) | 3;
    memory_info.PagePrev = (UInt32 *)(((UInt32)memory_info.PagePrev) + 4096);
}


void TzPagingInit() {
    memory_info.DirLocation = 0x400000;
    memory_info.PagingDir = (UInt32 *)(memory_info.DirLocation);
    memory_info.PagePrev = (UInt32 *)(memory_info.DirLocation + 0x4000);


    TzMemorySet(memory_info.PagingDir, 0 | 2, 1024);

    TzPagingMap(0, 0);
    TzPagingMap(0x400000, 0x400000);

    _asm_EnablePaging(memory_info.DirLocation);


}
*/





