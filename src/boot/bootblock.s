;==============================================================================;
;                               ictOS Boot Block                               ;
;                                                                      by: ict ;
;==============================================================================;

%include "sysboot.inc"
%include "bootblock.inc"

[section bootblock]
[bits 16]

    org BOOT_BLOCK_OFFSET

boot_block:
    xor     ax, ax
    mov     ds, ax
    mov     es, ax
    call    print
    db  "Boot...", 0xd, 0xa, 0x0
    call    init_fs_data
    mov     ax, KERNEL_BASE
    mov     es, ax
    mov     di, KERNEL_OFFSET
    mov     si, KernelFileName
    call    load_file
    test    ax, ax
    jz      no_kernel
    mov     ax, SYSLDR_BASE
    mov     es, ax
    mov     di, SYSLDR_OFFSET
    mov     si, SysldrFileName
    call    load_file
    test    ax, ax
    jz      no_sysldr
    cli
    mov     ax, SYSLDR_BASE
    mov     ds, ax
    mov     es, ax
    sti
    jmp     SYSLDR_BASE : SYSLDR_OFFSET
no_sysldr:
    call    print
    db  "Error: Missing [sysldr.ict]", 0xd, 0xa, 0x0
    jmp     boot_fail
no_kernel:
    call    print
    db  "Error: Missing [kernel.ict]", 0xd, 0xa, 0x0
    jmp     boot_fail

KernelFileName  db  'KERNEL  ICT'
SysldrFileName  db  'SYSLDR  ICT'

init_fs_data:
    xor     dx, dx
    xor     cx, cx
    mov     ax, word [BOOT_OFFSET + DBR_RESERVED_SECTORS]
    add     ax, word [BOOT_OFFSET + DBR_OFFSET]
    adc     dx, word [BOOT_OFFSET + DBR_OFFSET + 0x2]
    adc     cx, 0x0
    mov     word [FATOffset], ax
    mov     word [FATOffset + 0x2], dx
    mov     word [FATOffset + 0x4], cx
    xor     cx, cx
    mov     cl, byte [BOOT_OFFSET + DBR_FAT_SUM]
    cmp     cl, 0x2
    je      two_fat
one_fat:
    xor     cx, cx
    mov     ax, word [BOOT_OFFSET + DBR_FAT_SIZE]
    mov     dx, word [BOOT_OFFSET + DBR_FAT_SIZE + 0x2]
    jmp     comput_data_offset
two_fat:
    xor     cx, cx
    mov     ax, word [BOOT_OFFSET + DBR_FAT_SIZE]
    mov     dx, word [BOOT_OFFSET + DBR_FAT_SIZE + 0x2]
    shl     dx, 0x1
    adc     cx, 0x0
    shl     ax, 0x1
    adc     dx, 0x0
comput_data_offset:
    add     ax, word [BOOT_OFFSET + DBR_OFFSET]
    adc     dx, word [BOOT_OFFSET + DBR_OFFSET + 0x2]
    adc     cx, 0x0
    add     ax, word [BOOT_OFFSET + DBR_RESERVED_SECTORS]
    adc     dx, 0x0
    adc     cx, 0x0
    mov     word [DataOffset], ax
    mov     word [DataOffset + 0x2], dx
    mov     word [DataOffset + 0x4], cx
    ret

; ds:si -> filename
; es:di -> load address
load_file:
find_file:
    pusha
    push    di
    push    es
    mov     ax, ROOT_FAT_NUM
    mov     dx, 0x0
    mov     word [CurrentFATNumber], ax
    mov     word [CurrentFATNumber + 0x2], dx
    mov     word [main_index], dx
    mov     byte [block_index], ah
    xor     al, al
    xor     cx, cx
    xor     bx, bx
    shl     ax, 0x2
    mov     word [DAP_block_number], ax
    mov     word [DAP_block_number + 0x2], dx
    mov     word [DAP_block_number + 0x4], cx
    mov     word [DAP_block_number + 0x6], bx
    mov     ax, word [FATOffset]
    mov     dx, word [FATOffset + 0x2]
    mov     cx, word [FATOffset + 0x4]
    mov     bx, word [FATOffset + 0x6]
    add     word [DAP_block_number], ax
    adc     word [DAP_block_number + 0x2], dx
    adc     word [DAP_block_number + 0x4], cx
    adc     word [DAP_block_number + 0x6], bx
    mov     word [BufferOffset], FAT_BUFF_ADDR
    mov     word [BufferSegment], 0x0
    xor     ax, ax
    mov     al, FAT_BUFF_SIZE
    mov     word [BlockCount], ax
    mov     dl, byte [BOOT_OFFSET + DBR_DEVICE]
    mov     ah, 0x42
    push    si
    mov     si, DAP
    int     0x13
    pop     si
    jc      load_error
check_one_cluster:
    mov     word [BufferOffset], 0x0
    mov     word [BufferSegment], TMP_DATA_BASE
    call    load_cluster
    mov     ax, TMP_DATA_BASE
    mov     es, ax
    xor     ax, ax
check_filename:
    mov     di, ax
    mov     cx, 0xb ; len(filename) == 11
    push    si
    cld
    repz cmpsb
    pop     si
    jz      find_it
    add     ax, DE_SIZE
    cmp     ah, byte [BOOT_OFFSET + DBR_SECTORS_PER_CLUSTER]
    je      cluster_over
    mov     di, ax
    jmp     check_filename
cluster_over:
    call    refresh_fat_buff
    jc      check_one_cluster
    pop     es
    pop     di
    popa
    xor     ax, ax ; no such file
    ret
find_it:
    mov     si, ax
    mov     ax, word [es : si + DE_CLUSTER_LOW]
    mov     dx, word [es : si + DE_CLUSTER_HIGH]
    mov     word [CurrentFATNumber], ax
    mov     word [CurrentFATNumber + 0x2], dx
    pop     es
    pop     di  ; es:di -> load address
load_one_cluster:
    mov     word [BufferOffset], di
    mov     word [BufferSegment], es
    call    load_cluster
    xor     bx, bx
    mov     bl, byte [BOOT_OFFSET + DBR_SECTORS_PER_CLUSTER]
    shl     bx, 0x9
    add     di, bx
    test    di, di
    jnz     next_cluster
    push    ax
    mov     ax, es
    add     ax, 0x1000
    mov     es, ax
    pop     ax
next_cluster:
    call    refresh_fat_buff
    jc      load_one_cluster
    popa
    mov     ax, 0x1 ; loaded the file
    ret

refresh_fat_buff:
    pusha
    mov     ax, word [CurrentFATNumber]
    mov     dx, word [CurrentFATNumber + 0x2]
    cmp     dx, word [main_index]
    jne     load_fat_buff
    cmp     ah, byte [block_index]
    jne     load_fat_buff
    jmp     get_next_fat
init_fat_buff:
    inc     al
    mov     byte [InitFlag], al
    mov     ax, word [CurrentFATNumber]
    mov     dx, word [CurrentFATNumber + 0x2]
load_fat_buff:
    xor     al, al  ; clear offset
    xor     cx, cx
    xor     bx, bx
    shl     dx, 0x1
    adc     cx, bx
    shl     ax, 0x1
    adc     dx, bx
    shl     dx, 0x1
    adc     cx, bx
    shl     ax, 0x1
    adc     dx, bx
    mov     word [DAP_block_number], ax
    mov     word [DAP_block_number + 0x2], dx
    mov     word [DAP_block_number + 0x4], cx
    mov     word [DAP_block_number + 0x6], bx
    mov     ax, word [FATOffset]
    mov     dx, word [FATOffset + 0x2]
    mov     cx, word [FATOffset + 0x4]
    mov     bx, word [FATOffset + 0x6]
    add     word [DAP_block_number], ax
    adc     word [DAP_block_number + 0x2], dx
    adc     word [DAP_block_number + 0x4], cx
    adc     word [DAP_block_number + 0x6], bx
    mov     word [BufferOffset], FAT_BUFF_ADDR
    mov     word [BufferSegment], 0x0
    xor     ax, ax
    mov     al, FAT_BUFF_SIZE
    mov     word [BlockCount], ax
    mov     dl, byte [BOOT_OFFSET + DBR_DEVICE]
    mov     ah, 0x42
    mov     si, DAP
    int     0x13
    jc      load_error
get_next_fat:
    mov     bx, word [CurrentFATNumber]
    xor     bh, bh
    shl     bx, 0x2
    add     bx, FAT_BUFF_ADDR
    mov     ax, [bx]
    mov     dx, [bx + 0x2]
    mov     word [main_index], dx
    mov     byte [block_index], ah
    mov     word [CurrentFATNumber], ax
    mov     word [CurrentFATNumber + 0x2], dx
    cmp     ax, 0xffff
    jne     no_fat_link_over
    cmp     dx, 0x0fff
    jne     no_fat_link_over
    jmp     fat_link_over
no_fat_link_over:
    stc
    popa
    ret
fat_link_over:
    clc
    popa
    ret

InitFlag    db  0x0    

    ; [dx ax] == fat number
load_cluster:
    pusha
    mov     ax, word [CurrentFATNumber]
    mov     dx, word [CurrentFATNumber + 0x2]
    push    ax
    ; clear TempOffset
    xor ax, ax
    mov word [DAP_block_number], ax
    mov word [DAP_block_number + 0x2], ax
    mov word [DAP_block_number + 0x4], ax
    mov word [DAP_block_number + 0x6], ax
    mov cx, ax
    ; cx == 0x0
    ; comput offset by sectors
    pop     ax
    sub     ax, 0x2
    sbb     dx, 0x0
    mov     cl, byte [BOOT_OFFSET + DBR_SECTORS_PER_CLUSTER]
    mov     bx, dx
    mul     cx
    mov     word [DAP_block_number], ax
    mov     ax, bx
    mov     bx, dx
    mul     cx
    add     ax, bx
    adc     dx, 0x0
    mov     word [DAP_block_number + 0x2], ax
    mov     word [DAP_block_number + 0x4], dx
    mov     ax, word [DataOffset]
    mov     dx, word [DataOffset + 0x2]
    mov     cx, word [DataOffset + 0x4]
    mov     bx, word [DataOffset + 0x6]
    add     word [DAP_block_number], ax
    adc     word [DAP_block_number + 0x2], dx
    adc     word [DAP_block_number + 0x4], cx
    adc     word [DAP_block_number + 0x6], bx
    xor     ax, ax
    mov     al, byte [BOOT_OFFSET + DBR_SECTORS_PER_CLUSTER]
    mov     word [BlockCount], ax
    mov     dl, byte [BOOT_OFFSET + DBR_DEVICE]
    mov     ah, 0x42
    mov     si, DAP
    int     0x13
    jc      load_error
    popa
    ret

load_error:
    mov     si, error_number + 0x3
print_number:
    mov     al, ah
    and     al, 0x0f
    cmp     al, 0xa
    jb      print_digit
    add     al, 0x7
print_digit:
    add     byte [si], al
    dec     si
    mov     cl, 0x4
    shr     ah, cl
    jnz     print_number
    call    print
    db  "Read Disk Error: "
error_number:
    db  "0x00", 0xd, 0xa, 0x0

boot_fail:
    call    print
    db  "Boot Fail! Press Any key To Reboot...", 0x0
    xor     ah, ah
    int     0x16
    call    print
    db  0xd, 0xa, 0x0
    int     0x19    

print:    
    pop     si  ; pop ip to si
    cld ; clear direction flag
print_next:
    lodsb   ; load a char to al, si++
    test    al, al    ; if al == NULL
    jz      print_done  ; string is end
    mov     ah, 0x0e    ; set function number
    mov     bx, 0x0001  ; 0 page
    int     0x10    ; print a char
    jmp     print_next  ; print next char if it isn't NULL
print_done:
    jmp     si  ; now si point to the data after the string

FATOffset:
    fat_offset_low      dd  0x0
    fat_offset_high     dd  0x0

DataOffset:
    data_offset_low     dd  0x0
    data_offset_high    dd  0x0

CurrentFATNumber:       dd  0x0

BuffFATNumber:
    main_index          dw  0x0
    block_index         db  0x0

DAP:
    PacketSize          db  0x10    ; it always is 16 bytes
    Reserved            db  0x0
    BlockCount          dw  0x0
    BufferOffset        dw  0x0
    BufferSegment       dw  0x0
DAP_block_number:
    BlockNumLow         dd  0x0
    BlockNumHigh        dd  0x0
