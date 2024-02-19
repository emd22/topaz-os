.code32
.globl _b32_entry
.extern _start

.att_syntax prefix

# a small bootstrapper for our c third stage.

_b32_entry:
    # set up our segments and stack

    cli
    mov     $0x10, %ax
    mov     %ax, %ds
    mov     %ax, %es
    mov     %ax, %fs
    mov     %ax, %gs
    mov     %ax, %ss
    mov     $0x90000, %esp
    mov     %esp, %ebp
    push    %ebp


    call    _start

    mov $0xDEDBEF, %eax

    cli
    hlt


# void _asm_EnablePaging_(UInt32 PageDir)
.globl _asm_EnablePaging
.extern _asm_EnablePaging
_asm_EnablePaging:
    pop %eax
    mov %eax, %cr3

    mov %cr0, %eax
    orl $0x80000001, %eax
    mov %eax, %cr0


.globl _asm_GdtFlush
.extern _asm_GdtFlush
_asm_GdtFlush:
    mov 4(%esp), %eax
    lgdt (%eax)

    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss
    ljmp $0x08, $.flush
.flush:
    ret


.include "boot/interrupts.inc"
