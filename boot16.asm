[ORG 0]
[BITS 16]

entry16:
    jmp boot16
    nop

_bpb:
	bpbOem						db "TOPAZ OS"		; OEM name or version
	bpbBytesPerSector			dw 0			    ; Bytes per Sector (512)
	bpbSectorsPerCluster		db 0				; Sectors per cluster (usually 1)
	bpbReservedSectors			dw 0			    ; Reserved sectors
	bpbTotalFATs				db 0			    ; FAT copies
	bpbRootEntries				dw 0			    ; Root directory entries
	bpbFat12TotalSectors		dw 0			    ; Sectors in filesystem (0 for FAT16)
	bpbMediaDescriptor			db 0				; Media descriptor type (f0 for floppy or f8 for HDD)
	bpbSectorsPerFAT			dw 0			    ; Sectors per FAT
	bpbSectorsPerTrack			dw 0			    ; Sectors per track
	bpbHeadsPerCylinder			dw 0			    ; Heads per cylinder
	bpbHiddenSectors			dd 0		        ; Number of hidden sectors (0)
	bpbTotalSectors				dd 0		        ; Number of sectors in the filesystem
	bpbDriveNumber				db 0				; Sectors per FAT
	bpbCurrentHead				db 0				; Reserved (used to be current head)
	bpbSignature				db 0				; Extended signature (indicates we have serial, label, and type)
	bpbSerial					dd 0		        ; Serial number of partition
	bpbDiskLabel				db "TOPAZ  DISK"	; Volume label
	bpbFileSystem				db "FAT16   "		; Filesystem type

_data:
	dataSector                  dw 0				; Stores our data sector
	fileCluster                 dw 0				; Stores the cluster of the strap file
chs:
    chsTrack                    db 0				; Stores the track\cylinder for LBAToCHS
    chsHead                     db 0				; Stores the head for LBAToCHS
    chsSector                   db 0				; Stores the sector for LBAToCHS


print_msg:
    push si
    mov si, ErrorMsg
    call prints
    pop si
    call prints
    ret

prints:
    lodsb                ; load byte from string
    or      al, al	     ; check if we are at null terminator
    jz      prints.done	 ; end of string
    mov	    ah,	0eh	     ; al = character to print, ah = colour code
    int	    10h          ; call graphics interrupt
    jmp	    prints       ; repeat
prints.done:
    ret

boot16:
	cli
	mov     ax, 0x07c0							; set our segment registers to 0x00:0x07C0, our current location.
	mov     ds, ax                              ; note that our segment is our  memory location divided by 10.
	mov     es, ax
	mov     fs, ax
	mov     gs, ax

	mov     ax, 0								; set our stack to 0x00:0xFFFF
	mov     ss, ax                              ; this is the highest pointer of a single segment we can register
	mov     sp, 0xFFFF                          ; in real mode.
	sti

	mov     [bpbDriveNumber], dl                ; store drive number


    mov     ax, 50h                             ; move our extra segment to 50h, the location to store our
    mov     es, ax                              ; stage 2 image.

    mov     ax, 1                               ; load sector 1, the sector immediately after our current sector.
    mov     word[dataSector], ax                ; this should contain our second stage.

    mov     cx, 8                               ; read 8 sectors(4kb), this includes our second and third stage.

    mov     bx, 00h                             ; set our buffer location to be 50h:0
    call    read_sectors

    jmp     50h:00h                             ; jmp to stage 2

    cli
.0:
    hlt
    jmp .0

; lba_to_chs:
; inputs:
;   ax: lba address to convert to chs
; outputs:
;   chsTrack, chsHead, chsSector
lba_to_chs:
	; Based on the equation Sector = (LBA % Sectors per Track) + 1

	xor     dx, dx
	div     word[bpbSectorsPerTrack]				; Calculate the modulo (in DL)
	inc     dl
	mov     byte[chsSector], dl					; Store it
	
	; Based on the equation Head = (LBA / Sectors per Track) % Heads per Cylinder
	xor     dx, dx
	div     WORD [bpbHeadsPerCylinder]				; AX already contains LBA / Sectors per Track
	mov     byte[chsHead], dl
	
	; Based on the equation Track = LBA / (Sectors per Track * Number of Heads)
	mov     byte[chsTrack], al						; Very conveniently-placed AX; it already contains the output!
	
	ret

; read_sectors:
; inputs:
;   cx - sector count
;   ax - sector seek address
;   es:bx - destination address
read_sectors:
	mov     di, 5                               ; max amount of read attempts
	
read_sectors.loop:
    push    ax
    push    bx
    push    cx

    call    lba_to_chs

    mov     ah, 02h								; function -> read sectors
    mov     al, 1								; read one sector at a time, some bios implementations can get messy...
    mov     ch, byte[chsTrack]
    mov     cl, byte[chsSector]
    mov     dh, byte[chsHead]
    mov     dl, byte[bpbDriveNumber]
    int     13h
    jnc     read_sectors.success

    ; error while reading, reset disk and try again.
    mov     ah, 00h								; function -> reset disk
    int     13h									; reset disk

    dec     di									; decrement our attempt counter

    pop     cx
    pop     bx
    pop     ax

    jnz     read_sectors.loop					; try again
    jmp     read_sectors.fail					; we hit zero, must be an error.

read_sectors.success:
    ; DEBUG

    pop     cx
    pop     bx
    pop     ax

    add     bx, word[bpbBytesPerSector]		; Go to the next memory location
    inc     ax									; Read from the next sector
    loop    read_sectors

    mov     si, SecondStageMsg
    call    print_msg

    ret
read_sectors.fail:
    mov     si, ReadErrorMsg
    call    print_msg

    pop     cx
    pop     bx
    pop     ax

    int     18h
    ret

; Topaz bootloader stage 1
ErrorMsg        db "TZ.S1: ", 0

ReadErrorMsg    db "Disk read error", 13, 10, 0
SecondStageMsg  db "Read second stage", 13, 10, 0

times 510-($-$$) db 0
dw 0AA55h