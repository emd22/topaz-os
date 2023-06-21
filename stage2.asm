[ORG 0x500]
[BITS 16]

extern _start

%define IMAGE_PMODE_BASE 0x100000

_start:
    jmp     start

gdt_table:
gdt_null:
    dd      0x0000
    dd      0x0000
gdt_code:
    dw      0xFFFF ; code limit, 4GB
    dw      0x0000 ; base address
    db      0x00
    db      0x9A
    db      0xCF
    db      0x00
gdt_data:
    dw      0xFFFF
    dw      0x0000
    db      0x00
    db      0x92
    db      0xCF
    db      0x00

    dw      0xFFFF
    dw      0x0000
    db      0x00
    db      0x9E
    db      0x00
    db      0x00

    dw      0xFFFF
    dw      0x0000
    db      0x00
    db      0x92
    db      0x00
    db      0x00

gdt_end:
gdt_descriptor:
    dw      gdt_end-gdt_table-1
    dd      gdt_table

CODE_SEGMENT    equ gdt_code - gdt_table
DATA_SEGMENT    equ gdt_data - gdt_table

prints:
	lodsb                ; load byte from string
	or      al, al	     ; check if we are at null terminator
	jz      prints.done	 ; end of string
	mov	    ah,	0eh	     ; al = character to print, ah = colour code
	int	    10h          ; call graphics interrupt
	jmp	    prints       ; repeat
prints.done:
	ret

start:
	cli				; clear interrupts
    xor	    ax, ax			; null segments
    mov	    ds, ax
    mov	    es, ax
    mov	    ax, 0x9000		; stack begins at 0x9000-0xffff
    mov	    ss, ax
    mov	    sp, 0xFFFF
    sti				; enable interrupts


	mov     si, StartMessage
	call    prints

a20_activate:
    cli
    mov     ax, 0x2401
    int     0x15
    jb      a20_error
    cmp     ah, 0
    jnz     a20_error
    sti

; enter protected mode
pmode_switch:
    cli
    lgdt     [gdt_descriptor]
    sti

    cli
    mov     eax, cr0        ; move cr0 into eax (load cr0)
    or      eax, 0x1        ; set protected mode bit
    mov     cr0, eax        ; set cr0 to eax (store new cr0)

    ; long jump to 0x10:pmode_entry offset
    jmp     CODE_SEGMENT:pmode_entry

    ; jump failed, halt
halt:
    hlt
    jmp     halt

a20_error:
    mov     si, A20_ERROR_MSG
    call    prints
    cli
.1:
    hlt
    jmp     .1

BootMessage     db "in 2nd stage bootloader", 13, 10, 0
A20_ERROR_MSG   db "E.A20", 13, 10, 0

[BITS 32]
pmode_entry:
    mov     ax, DATA_SEGMENT
    mov     ds, ax
    mov     ss, ax
    mov     es, ax
    mov     esp, 0x90000

.copy_boot32:
    mov     eax, 512
    mov     ebx, 7
    mul     ebx

    cld
    mov     esi, 0x700
    mov	    edi, IMAGE_PMODE_BASE
    mov	    ecx, eax
    rep	    movsd                   ; copy image to its protected mode address
.jump_boot32:
    mov     ebp, IMAGE_PMODE_BASE
    call    ebp

    cli
    hlt

StartMessage	db	13, 10, "* IN STAGE2 *", 13, 10, 0

