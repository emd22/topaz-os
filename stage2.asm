[BITS 16]
[ORG 0x100]

_entry:
    mov si, BootMessage
    call prints
halt:
    hlt
    jmp halt

prints:
    cld
    mov     ah, 0x0E
    mov     bx, 7
.1:
    lodsb
    test    al, al
    jz      .2
    int     0x10
    jmp     .1
.2:
    ret

BootMessage db "Made it to second stage!", 13, 10, 0
