#ifndef TOPAZ_INTERRUPT_H
#define TOPAZ_INTERRUPT_H

#include <Types.h>
#include <ArchX86.h>

#include <boot/IntAsm.h>

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

typedef void (*TzIrqHandle)(TzRegisterList *);

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

void TzGdtSetGate(Int32 n, UInt32 base, UInt32 limit, UInt8 access, UInt8 gran);
void TzGdtInit();

void TzIdtSetGate(Int8 num, UInt32 base, UInt16 selector, UInt8 flags);
void TzRegisterIrq(UInt8 int_no, TzIrqHandle handler);
void TzSetupIrqs();
void TzIdtInit();

#endif