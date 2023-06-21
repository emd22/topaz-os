.code32
.globl _b32_entry
.extern _start

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

    cli
    hlt
