#ifndef TOPAZ_ARCH_X86_H
#define TOPAZ_ARCH_X86_H

#include <Types.h>

typedef struct {
    UInt32 ds;
    UInt32 edi, esi, ebp, esp, ebx, edx, ecx, eax;
    UInt32 int_no, err_code;
    UInt32 eip, cs, eflags, useresp, ss;
} TzRegisterList;


#define TZ_ASM(str) { __asm__ volatile (str "\n"); }


#define TzInterruptsDisable() { TZ_ASM("cli"); }
#define TzInterruptsEnable()  { TZ_ASM("sti"); }

#define TzHalt() {         \
    TzInterruptsDisable(); \
    TZ_ASM("hlt");         \
}

inline void TzOut8(UInt16 port, UInt8 value) {
    __asm__("outb %0, %1" : : "a"(value), "Nd"(port) : "memory");
}

inline UInt8 TzIn8(UInt16 port) {
    UInt8 value;
    __asm__("inb %1, %0" : "=a"(value) : "Nd"(port) : "memory");
    return value;
}


inline void TzOut32(UInt16 port, UInt32 value) {
    __asm__("outl %0, %1" : : "a"(value), "Nd"(port) : "memory");
}

inline UInt32 TzIn32(UInt16 port) {
    UInt32 value;
    __asm__("inl %1, %0" : "=a"(value) : "Nd"(port) : "memory");
    return value;
}



#endif