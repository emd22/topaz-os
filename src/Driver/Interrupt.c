#ifndef TOPAZ_INTERRUPTS_H
#define TOPAZ_INTERRUPTS_H

#include <Driver/TtyGraphics.h>

#include "Interrupt.h"
#include <TzKern.h>

#include "TzString.h"

extern void _asm_GdtFlush(UInt32 gdtp);
extern void _asm_IdtFlush(UInt32 idtp);

TzGdtEntry gdt_entries[5];
TzGdtPtr gdt_ptr;

TzIdtEntry idt_entries[256];
TzIdtPtr   idt_ptr;

TzIrqHandle interrupt_handlers[256];

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
    gdt_ptr.limit = (sizeof(TzGdtEntry) * 5) - 1;
    gdt_ptr.base = (UInt32)&gdt_entries;

    TzGdtSetGate(0, 0, 0, 0, 0);                // null segment
    TzGdtSetGate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // code segment
    TzGdtSetGate(2, 0, 0xFFFFFFFF, 0x92, 0xCF); // data segment
    TzGdtSetGate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF); // user mode code segment
    TzGdtSetGate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF); // user mode data segment

    _asm_GdtFlush((UInt32)&gdt_ptr);
}

void TzIdtSetGate(Int8 num, UInt32 base, UInt16 selector, UInt8 flags) {
    TzIdtEntry *entry = &idt_entries[num];
    entry->base_lo  = (base & 0xFFFF);
    entry->base_hi  = (base >> 16) & 0xFFFF;
    entry->selector = selector;
    entry->_resv0 = 0;
    // TODO: when we get to user mode, OR(|) flags with 0x60.
    entry->flags = flags;
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
        handler(&registers);
    }

}

extern void TzIdtFaultHandler(TzRegisterList registers) {
    TzIrqHandle *handler = &interrupt_handlers[registers.int_no];

    if (*handler) {
        (*handler)(&registers);
        return;
    }
    TzPanic("Unhandled ISR Fault 0x%x", registers.int_no);
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

}

void TzIdtInit() {
    TzInterruptsDisable();

    TzMemorySet(&interrupt_handlers, 0, sizeof(TzIrqHandler)*256);

    idt_ptr.limit = sizeof(TzIdtEntry) * 256 - 1;
    idt_ptr.base = (UInt32)&idt_entries;

    // Clear our entries
    TzMemorySet(&idt_entries, 0, sizeof(TzIdtEntry) * 256);

    TzSetupIrqs();

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


    _asm_IdtFlush((UInt32)&idt_ptr);

    TzInterruptsEnable();
}


#endif