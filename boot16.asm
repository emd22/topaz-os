[BITS 16]
[CPU 386]

ImageLoadSeg            equ 0x60
StackSize               equ 2048

; origin 0
ORG 0

base_address:
bsDriveNumber:
    jmp short start16
    nop

bsOemName db "TOPAZ OS"

; BPB table
bpbBytesPerSector          dw 0
bpbSectorsPerCluster       db 0
bpbReservedSectors         dw 0
bpbNumberOfFATs            db 0
bpbRootEntries             dw 0
bpbTotalSectors            dw 0
bpbMedia                   db 0
bpbSectorsPerFAT           dw 0
bpbSectorsPerTrack         dw 0
bpbHeadsPerCylinder        dw 0
bpbHiddenSectors           dd 0
bpbTotalSectorsBig         dd 0

; EBPB table
ebpbSectorsPerFAT32        dd 0
ebpbExtendedFlags          dw 0
ebpbFSVersion              dw 0
ebpbRootDirectoryClusterNo dd 0
ebpbFSInfoSectorNo         dw 0
ebpbBackupBootSectorNo     dw 0

; 12 bytes of reserved space
ebpbResv0 dq 0
ebpbResv1 dd 0

ebpbDriveNumber            db 0
ebpbResv2                  db 0
ebpbExtendedBootSignature  db 0
ebpbVolumeSerialNumber     dd 0
ebpbVolumeLabel            db "NO NAME    "
ebpbFileSystemName         db "FAT32   "

; real mode entry point (this is our start)
start16:
    cld ; clear memory direction bit

    int     0x12  ; get memory size in kb
    shl     ax, 6 ; convert size to to 16-byte paragraphs

    ; reserve memory for our bootstrap and our stack, divide by 16 to get our segment.
    ; Segment = (TopOfMemory - (512 + OurStackSize)) / 16

    sub     ax, (512 + StackSize) / 16
    mov     es, ax                  ; segment for extra data
    mov     ss, ax                  ; setup our stack segment

    ; stack starts at the top, adjust to our stack size + size of our sector

    mov     sp, 512 + StackSize     ; setup our stack pointer

    ; copy our sector to the top of memory, to avoid memory conflicts

    mov     cx, 256                 ; repeat word copy 256 times
    mov     si, 7C00h               ; memory source
    xor     di, di
    mov     ds, di
    rep     movsw

    ; jump to the copy
    push    es
    push    byte boot16             ; push our new jump point
    retf                            ; jump to our boot16(this also refreshes segments)

boot16:
    push    cs
    pop     ds ; ds = cs

    mov     [bsDriveNumber], dl     ; store boot drive

    ; check for drive extensions for LBA disk operations
    mov     ah, 41h
    mov     bx, 0x55AA
    int     13h
    jc      no_extensions
    sub     bx, 0xAA55
    jnz     no_extensions
    shr     cx, 1
    jc      found_extensions

no_extensions:
    jmp     read_error

found_extensions:
    ; bx = 0, es:bx is our buffer for root dir and stage 2 file
    push    ImageLoadSeg
    pop     es


    ; load all clusters of the root dir
load_root_dir:
    and     byte [bx+(ebpbRootDirectoryClusterNo-base_address)+3], 0Fh ; mask cluster value
    mov     ebp, [bx+(ebpbRootDirectoryClusterNo-base_address)]        ; ebp=cluster # of root dir

root_dir_read_next:
    push    es
    call    read_cluster             ; read one cluster of root dir ; cx = 0
    pop     es
    pushf                            ; save carry="not last cluster" flag

    ; es:di is the root entries buffer
    mov     di, bx

    ; move and zero extend our byte
    movzx   ax, byte [bx+(bpbSectorsPerCluster-base_address)]
    ; ax is our bytes per cluster
    mul     word [bx+(bpbBytesPerSector-base_address)]

find_file:
    ; ds:si is the program name
    mov     si, ProgramName         ; ds:si -> program name
    ; 11 characters for a fat32 filename
    mov     cl, 11
    ; we are at the end of our root directory, file not found
    cmp     byte [es:di], ch
    je      file_not_found

    ; string compare
    push    di
    repe    cmpsb
    pop     di
    ; found file, go to next
    je      file_found
    ; go to next dir entry
    add     di, 32
    ; check if we are at end
    cmp     di, ax
    ; are not, go to next file
    jne     find_file

    ; restore carry flag since we are not at last cluster
    popf
    ; go to next cluster of root dir
    jc      root_dir_read_next
file_not_found:
    call    error
    db      "E.NOFILE", 0
file_found:
    push    word [es:di + 0x14]
    push    word [es:di + 0x1A]
    pop     ebp                     ; ebp = file cluster no.

    push    es

    ; load our file

file_read_cluster:
    call    read_cluster             ; read one cluster of file
    ; not EOF, continue reading clusters
    jc file_read_cluster

    mov     dl, [bx+(bsDriveNumber-base_address)] ; pass the BIOS boot drive

    pop     ax                      ; ImageLoadSeg
    mov     ds, ax                  ; ax=ds=seg the file is loaded to

    sub     ax, 10h                 ; "org 100h" stuff :)
    mov     es, ax
    mov     ds, ax
    mov     ss, ax
    xor     sp, sp
    mov     bh, 1                   ; ax:bx = cs:ip of entry point
    jmp     short enter_stage2

enter_stage2:
    push    ax
    push    bx

    ; jump to our stage 2 bootloader!
    retf

; read_cluster:
;
; input:
;   es:bx -> pointer to buffer
;   ebp   -> cluster number to read
; output:
;   ebp   -> next cluster
;   es    -> next address segment

read_cluster:
    lea     esi, [ebp-2]                    ; esi=prev cluster # - 2

    mov     ax, [bx+(bpbBytesPerSector-base_address)]
    shr     ax, 2                           ; ax=# of FAT32 entries per sector
    cwde                                    ; eax=# of FAT32 entries per sector
    xchg    eax, ebp
    cdq
    div     ebp                             ; eax=FAT sector #, edx=entry # in sector

    mov     cx, 1
    push    es
    call    read_sector                     ; read 1 FAT32 sector
    pop     es

    imul    di, dx, 4
    and     byte [es:bx+di+3], 0Fh          ; mask cluster value
    mov     ebp, [es:bx+di]                 ; ebp=next cluster #

    ; get the size of our FAT
    movzx   eax, byte [bx+(bpbNumberOfFATs-base_address)]
    mul     dword [bx+(ebpbSectorsPerFAT32-base_address)]

    xchg    eax, esi

    movzx   ecx, byte [bx+(bpbSectorsPerCluster-base_address)] ; ecx = sector count
    mul     ecx

    add     eax, esi                        ; eax=LBA (relative to 1st FAT start)

; read_sector:
;
; input:
;   eax   -> lba
;   cx    -> sector count
;   es:bx -> buffer pointer
; output:
;   eax   -> next lba
;   es    -> next address segment
read_sector:
    ; max 5 retry attempts
    mov     di, 5

read_sector_try:
    pushad

    ; we use 32 bit registers here so we don't overflow
    movzx   edx, word [bx+(bpbReservedSectors-base_address)]
    add     eax, edx

    xor     dx, dx
    add     eax, [bx+(bpbHiddenSectors-base_address)]
    adc     dx, bx

    push    edx                     ; lba high bits
    push    eax                     ; lba low bits
    push    es                      ; memory buffer segment
    push    bx                      ; memory buffer offset
    push    byte 1                  ; sector count
    ; reserved, set to 0
    push    byte 0x10 ; DAP size 16

    mov     si, sp                  ; ds:si is our DAP

    mov     ah, 0x42                ; 0x42 -> read sectors by LBA addressing
    mov     dl, [bx+(bsDriveNumber-base_address)]
    int     0x13                    ; read sectors

    lea     sp, [si+16]             ; remove DAP from stack

    jnc     read_sector_done        ; carry is zero if there is no error
    mov     ah, 0                   ; set function to disk reset
    int     0x13                    ; reset drive

    popad

    ; first attempt failed, retry.
    ; this used to be very important in the days of floppy disks, and
    ; even in the days of CD-ROMs. Not really as necessary anymore, but
    ; better to be safe.
    dec     di
    jnz     short read_sector_try

read_error:
    call    error
    db      "E.READ", 0

read_sector_done:
    mov     dx, [bx+(bpbBytesPerSector-base_address)]
    shr     dx, 4                   ; dx = sector size in paragraphs
    mov     ax, es
    add     ax, dx
    mov     es, ax                  ; es updated

    popad

    inc     eax                     ; adjust LBA for next sector

    loop    read_sector             ; if not last sector

read_cluster_continue:
    cmp     ebp, 0FFFFFF8h          ; carry is set to zero if we are at our last cluster

    ret

error:
    ; get our string pointer, set up for printing
    pop     si
    mov     ah, 0x0E
    mov     bx, 7

error_next:
    lodsb
.1:
    hlt
    test    al, al
    jz      .1
    int     0x10
    jmp     short error_next

ProgramName db "STAGE2  BIN"

; pad extra space
times (510-($-$$)) db 0

; boot sector signature
dw 0xAA55
