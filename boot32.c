#include "src/Types.h"
#include "Interrupt.h"

#define NULL (void *)0

void printc(char ch, int color);
void prints(const char *str, int color);
void kmain(void);

void TzPagingMap(UInt32 virt, UInt32 phys);
void TzPagingInit();

extern void _asm_EnablePaging(UInt32 page_directory);
extern void _asm_GdtFlush(UInt32 gdtp);
extern void _asm_IdtFlush(UInt32 idtp);

void TzGdtInit();
void TzIdtInit();

inline void TzOut8(UInt16 port, UInt8 value);
inline UInt8 TzIn8(UInt16 port);

void TzRegisterIrq(UInt8 int_no, TzIrqHandle handler);

// start of our 32 bit bootloader
extern void _start(void) {
    kmain();

    __asm__("cli");
    __asm__("hlt");
}

typedef enum {
    TZ_COLOR_BLACK,
    TZ_COLOR_BLUE,
    TZ_COLOR_GREEN,
    TZ_COLOR_CYAN,
    TZ_COLOR_RED,
    TZ_COLOR_PURPLE,
    TZ_COLOR_BROWN,
    TZ_COLOR_GRAY,
    TZ_COLOR_DARK_GRAY,
    TZ_COLOR_LIGHT_BLUE,
    TZ_COLOR_LIGHT_GREEN,
    TZ_COLOR_LIGHT_CYAN,
    TZ_COLOR_LIGHT_RED,
    TZ_COLOR_LIGHT_PURPLE,
    TZ_COLOR_YELLOW,
    TZ_COLOR_WHITE
} TzTextColor;

typedef struct {
    TzTextColor background_color;
    TzTextColor foreground_color;
    Int buffer_offset;
} TzGraphicsState;

static TzGraphicsState graphics_state;

void TzBackgroundColorSet(TzTextColor color) {
    graphics_state.background_color = color;
}

TzTextColor TzGetBackgroundColor() {
    return graphics_state.background_color;
}

void TzForegroundColorSet(TzTextColor color) {
    graphics_state.foreground_color = color;
}

TzTextColor TzGetForegroundColor() {
    return graphics_state.foreground_color;
}


void TzDisplayClear() {
    int i;
    for (i = 0; i < 80 * 25; i++) {
        printc(' ', 0 | (TzGetBackgroundColor() << 4));
    }
}

void TzSetPosition(Int x, Int y) {
    graphics_state.buffer_offset = (y * 80) + x;
}

Int TzGetPosition() {
    return graphics_state.buffer_offset;
}

void TzPrintString(Char *str) {
    prints(str, (TzGetBackgroundColor() << 4) | (TzGetForegroundColor() & 0x0F));
}


void TzTimerCallback(TzRegisterList registers) {
    TzPrintString("x");
}


void TzInitTimer(UInt32 freq) {
    TzRegisterIrq(TZ_IRQ0, TzTimerCallback);

    UInt32 divisor = 1193180 / freq;

    // send command
    TzOut8(0x43, 0x36);

    UInt8 lo = (UInt8)(divisor & 0xFF);
    UInt8 hi = (UInt8)((divisor >> 8) & 0xFF);

    TzOut8(0x40, lo);
    TzOut8(0x40, hi);
}





void kmain(void) {
    TzBackgroundColorSet(TZ_COLOR_CYAN);
    TzForegroundColorSet(TZ_COLOR_YELLOW);
    TzDisplayClear();
    TzSetPosition(0, 0);
    TzPrintString("Welcome to TOPAZ OS!\n");
    TzForegroundColorSet(TZ_COLOR_WHITE);
    TzPrintString("Setting up descriptors: ");

    TzGdtInit();
    TzPrintString("GDT [OK] ");
    TzIdtInit();
    TzPrintString("IDT [OK]\n");

    __asm__("sti\n");

    TzInitTimer(50);
    TzPrintString("TIMER [OK]\n");


//    TzPagingInit();
}

void *TzMemorySet(void *ptr, Int v, UInt32 n) {
    if (!n)
        return ptr;

    Char *p = ptr;
    do {
        *p++ = v;
    } while(--n);

    return ptr;
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


typedef struct {
    UInt16 limit_lo;
    UInt16 base_lo;
    UInt8  base_mid;
    UInt8  access;
    UInt8  granularity;
    UInt8  base_hi;
} TZ_PACK TzGdtEntry;

typedef struct {
    UInt16 limit;
    UInt32 base;
} TZ_PACK TzGdtPtr;

TzGdtEntry gdt_entries[5];
TzGdtPtr gdt_ptr;

void TzGdtSetGate(Int32 n, UInt32 base, UInt32 limit, UInt8 access, UInt8 gran) {
    TzGdtEntry *entry = &gdt_entries[n];
    entry->base_lo  = (base & 0xFFFF);
    entry->base_mid = (base >> 16) & 0xFF;
    entry->base_hi  = (base >> 24) & 0xFF;
    entry->limit_lo = (limit & 0xFFFF);
    entry->granularity = (limit >> 16) & 0x0F;
    entry->granularity |= gran & 0xF0;
    entry->access = access;
}


void TzGdtInit() {
    gdt_ptr.limit = sizeof(TzGdtEntry) * 5 - 1;
    gdt_ptr.base = (UInt32)&gdt_entries;

    TzGdtSetGate(0, 0, 0, 0, 0);                // null segment
    TzGdtSetGate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // code segment
    TzGdtSetGate(2, 0, 0xFFFFFFFF, 0x92, 0xCF); // data segment
    TzGdtSetGate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF); // user mode code segment
    TzGdtSetGate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF); // user mode data segment

    _asm_GdtFlush((UInt32)&gdt_ptr);
}


typedef struct {
    UInt16 base_lo;
    UInt16 selector;
    UInt8  _resv0;
    UInt8  flags;
    UInt16 base_hi;
} TZ_PACK TzIdtEntry;

typedef struct {
    UInt16 limit;
    UInt32 base;
} TZ_PACK TzIdtPtr;

TzIdtEntry idt_entries[256];
TzIdtPtr   idt_ptr;

TzIrqHandle interrupt_handlers[256];


void TzIdtSetGate(Int8 num, UInt32 base, UInt16 selector, UInt8 flags) {
    TzIdtEntry *entry = &idt_entries[num];
    entry->base_lo  = (base & 0xFFFF);
    entry->base_hi  = (base >> 16) & 0xFFFF;
    entry->selector = selector;
    entry->_resv0 = 0;
    // TODO: when we get to user mode, OR(|) flags with 0x60.
    entry->flags = flags;
}


inline void TzOut8(UInt16 port, UInt8 value) {
    __asm__("outb %0, %1" : : "a"(value), "Nd"(port) : "memory");
}

inline UInt8 TzIn8(UInt16 port) {
    UInt8 value;
    __asm__("inb %1, %0" : "=a"(value) : "Nd"(port) : "memory");
    return value;
}

void TzRegisterIrq(UInt8 int_no, TzIrqHandle handler) {
    interrupt_handlers[int_no] = handler;
}


extern void TzIrqHandler(TzRegisterList registers) {
    if (registers.int_no >= 40) {
        // Reset slave
        TzOut8(0xA0, 0x20);
    }
    // Reset master
    TzOut8(0x20, 0x20);

    if (interrupt_handlers[registers.int_no] != 0) {
        TzIrqHandle handler = interrupt_handlers[registers.int_no];
        handler(registers);
    }

}

extern void TzIdtFaultHandler(TzRegisterList registers) {
    __asm__("cli\n");
    __asm__("sti\n");
}

void TzSetupIrqs() {
    // Remap the IDT!
    TzOut8(0x20, 0x11);
    TzOut8(0xA0, 0x11);
    TzOut8(0x21, 0x20);
    TzOut8(0xA1, 0x28);
    TzOut8(0x21, 0x04);
    TzOut8(0xA1, 0x02);
    TzOut8(0x21, 0x01);
    TzOut8(0xA1, 0x01);
    TzOut8(0x21, 0x00);
    TzOut8(0xA1, 0x00);

    // Setup our IRQ gates
    TzIdtSetGate(32, (UInt32)_irq0, 0x08, 0x8E);
    TzIdtSetGate(33, (UInt32)_irq1, 0x08, 0x8E);
    TzIdtSetGate(34, (UInt32)_irq2, 0x08, 0x8E);
    TzIdtSetGate(35, (UInt32)_irq3, 0x08, 0x8E);
    TzIdtSetGate(36, (UInt32)_irq4, 0x08, 0x8E);
    TzIdtSetGate(37, (UInt32)_irq5, 0x08, 0x8E);
    TzIdtSetGate(38, (UInt32)_irq6, 0x08, 0x8E);
    TzIdtSetGate(39, (UInt32)_irq7, 0x08, 0x8E);
    TzIdtSetGate(40, (UInt32)_irq8, 0x08, 0x8E);
    TzIdtSetGate(41, (UInt32)_irq9, 0x08, 0x8E);
    TzIdtSetGate(42, (UInt32)_irq10, 0x08, 0x8E);
    TzIdtSetGate(43, (UInt32)_irq11, 0x08, 0x8E);
    TzIdtSetGate(44, (UInt32)_irq12, 0x08, 0x8E);
    TzIdtSetGate(45, (UInt32)_irq13, 0x08, 0x8E);
    TzIdtSetGate(46, (UInt32)_irq14, 0x08, 0x8E);
    TzIdtSetGate(47, (UInt32)_irq15, 0x08, 0x8E);

}

void TzIdtInit() {
    idt_ptr.limit = sizeof(TzIdtEntry) * 256 - 1;
    idt_ptr.base = (UInt32)&idt_entries;

    // Clear our entries
    TzMemorySet(&idt_entries, 0, sizeof(TzIdtEntry) * 256);

    // Reset all gates
    TzIdtSetGate(0, (UInt32)_isr0, 0x08, 0x8E);
    TzIdtSetGate(1, (UInt32)_isr1, 0x08, 0x8E);
    TzIdtSetGate(2, (UInt32)_isr2, 0x08, 0x8E);
    TzIdtSetGate(3, (UInt32)_isr3, 0x08, 0x8E);
    TzIdtSetGate(4, (UInt32)_isr4, 0x08, 0x8E);
    TzIdtSetGate(5, (UInt32)_isr5, 0x08, 0x8E);
    TzIdtSetGate(6, (UInt32)_isr6, 0x08, 0x8E);
    TzIdtSetGate(7, (UInt32)_isr7, 0x08, 0x8E);
    TzIdtSetGate(8, (UInt32)_isr8, 0x08, 0x8E);
    TzIdtSetGate(9, (UInt32)_isr9, 0x08, 0x8E);
    TzIdtSetGate(10, (UInt32)_isr10, 0x08, 0x8E);
    TzIdtSetGate(11, (UInt32)_isr11, 0x08, 0x8E);
    TzIdtSetGate(12, (UInt32)_isr12, 0x08, 0x8E);
    TzIdtSetGate(13, (UInt32)_isr13, 0x08, 0x8E);
    TzIdtSetGate(14, (UInt32)_isr14, 0x08, 0x8E);
    TzIdtSetGate(15, (UInt32)_isr15, 0x08, 0x8E);
    TzIdtSetGate(16, (UInt32)_isr16, 0x08, 0x8E);
    TzIdtSetGate(17, (UInt32)_isr17, 0x08, 0x8E);
    TzIdtSetGate(18, (UInt32)_isr18, 0x08, 0x8E);
    TzIdtSetGate(19, (UInt32)_isr19, 0x08, 0x8E);
    TzIdtSetGate(20, (UInt32)_isr20, 0x08, 0x8E);
    TzIdtSetGate(21, (UInt32)_isr21, 0x08, 0x8E);
    TzIdtSetGate(22, (UInt32)_isr22, 0x08, 0x8E);
    TzIdtSetGate(23, (UInt32)_isr23, 0x08, 0x8E);
    TzIdtSetGate(24, (UInt32)_isr24, 0x08, 0x8E);
    TzIdtSetGate(25, (UInt32)_isr25, 0x08, 0x8E);
    TzIdtSetGate(26, (UInt32)_isr26, 0x08, 0x8E);
    TzIdtSetGate(27, (UInt32)_isr28, 0x08, 0x8E);
    TzIdtSetGate(28, (UInt32)_isr28, 0x08, 0x8E);
    TzIdtSetGate(29, (UInt32)_isr29, 0x08, 0x8E);
    TzIdtSetGate(30, (UInt32)_isr30, 0x08, 0x8E);
    TzIdtSetGate(31, (UInt32)_isr31, 0x08, 0x8E);

    TzSetupIrqs();

    _asm_IdtFlush((UInt32)&idt_ptr);
}

void printc(char ch, int color) {
    if (ch == '\n') {
        TzSetPosition(0, TzGetPosition() / 80 + 1);
        return;
    }
    unsigned long video_addr = (0xB8000 + (TzGetPosition() * 2));
    char *video_buffer = (char *)video_addr;
    *(video_buffer++) = ch;
    *(video_buffer++) = color;
    graphics_state.buffer_offset++;

}

void prints(const char *_str, int color) {
    int i = 0;
    char ch;
    for (i = 0; (ch = _str[i]); i++) {

        printc(ch, color);
    }
}