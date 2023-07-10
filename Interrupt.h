#ifndef TOPAZ_OS_ISR_H
#define TOPAZ_OS_ISR_H


#include "src/Types.h"

typedef enum {
    TZ_IRQ0 = 32,
    TZ_IRQ1,
    TZ_IRQ2,
    TZ_IRQ3,
    TZ_IRQ4,
    TZ_IRQ5,
    TZ_IRQ6,
    TZ_IRQ7,
    TZ_IRQ8,
    TZ_IRQ9,
    TZ_IRQ10,
    TZ_IRQ11,
    TZ_IRQ12,
    TZ_IRQ13,
    TZ_IRQ14,
    TZ_IRQ15,
} TzIrqNo;


typedef struct {
    UInt32 ds;
    UInt32 edi, esi, ebp, esp, ebx, edx, ecx, eax;
    UInt32 int_no, err_code;
    UInt32 eip, cs, eflags, useresp, ss;
} TZ_PACK TzRegisterList;

typedef void (*TzIrqHandle)(TzRegisterList);

extern void _isr0();
extern void _isr1();
extern void _isr2();
extern void _isr3();
extern void _isr4();
extern void _isr5();
extern void _isr6();
extern void _isr7();
extern void _isr8();
extern void _isr9();
extern void _isr10();
extern void _isr11();
extern void _isr12();
extern void _isr13();
extern void _isr14();
extern void _isr15();
extern void _isr16();
extern void _isr17();
extern void _isr18();
extern void _isr19();
extern void _isr20();
extern void _isr21();
extern void _isr22();
extern void _isr23();
extern void _isr24();
extern void _isr25();
extern void _isr26();
extern void _isr27();
extern void _isr28();
extern void _isr29();
extern void _isr30();
extern void _isr31();

extern void _irq0();
extern void _irq1();
extern void _irq2();
extern void _irq3();
extern void _irq4();
extern void _irq5();
extern void _irq6();
extern void _irq7();
extern void _irq8();
extern void _irq9();
extern void _irq10();
extern void _irq11();
extern void _irq12();
extern void _irq13();
extern void _irq14();
extern void _irq15();

#endif