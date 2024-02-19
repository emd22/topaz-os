#include "Pci.h"
#include "Types.h"

#include "ArchX86.h"


UInt16 TzPciConfigReadWord(UInt8 bus, UInt8 slot, UInt8 func, UInt8 offset) {
    UInt32 addr;
    UInt32 lbus = (UInt32)bus;
    UInt32 lslot = (UInt32)slot;
    UInt32 lfunc = (UInt32)func;
    UInt16 temp = 0;

    addr = (UInt32)((lbus << 16) | (lslot << 11) | (lfunc << 8) | (offset & 0xFC) | ((UInt32)0x80000000));
    TzOut32(0x0CF8, addr);

    temp = (UInt16)((TzIn32(0x0CFC) >> ((offset & 2) * 8)) & 0xFFFF);

    return temp;
}

UInt16 TzPciCheckVendor(UInt8 bus, UInt8 slot) {
    UInt16 vendor, device;

    if ((vendor = TzPciConfigReadWord(bus, slot, 0, 0)) != 0xFFFF) {
        device = TzPciConfigReadWord(bus, slot, 0, 2);
    }

    return vendor;
}



