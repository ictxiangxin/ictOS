;==============================================================================;
;                             ictOS 1.00 sysboot                               ;
;                                                                      by: ict ;
;==============================================================================;

%include    "sysboot.inc"

[section sysboot]
[bits 16]

    org     BOOT_OFFSET
    jmp     start_boot
    nop

%include    "../fs/fat32dbr.inc"

start_boot:
    xor     ax, ax
    mov     ds, ax
    mov     es, ax
    cli
    mov     sp, BOOT_OFFSET
    mov     ss, ax
    sti
    mov     ax, 0x3
    int     0x10
    mov     ax, 0x600
    mov     bx, 0x700
    mov     cx, 0x0
    mov     dx, 0x184f
    int     0x10
    mov     dx, 0x0
    mov     bh, 0x0
    mov     ah, 0x2
    int     0x10
    
    call    print
    db  "ictOS", 0xd, 0xa, 0x0

    mov     di, 0x3 ; try 3 times
load_boot_block:
    mov     ax, BOOT_BLOCK_ADDR
    xor     bx, bx
    xor     cx, cx
    xor     dx, dx
    add     ax, word [dbr_offset]
    adc     bx, word [dbr_offset + 0x2]
    adc     cx, 0x0
    mov     word [BlockNumLow], ax
    mov     word [BlockNumLow + 0x2], bx
    mov     word [BlockNumHigh], cx
    mov     word [BlockNumHigh + 0x2], dx
    mov     word [BufferOffset], BOOT_BLOCK_OFFSET
    mov     word [BlockCount], BOOT_BLOCK_SIZE
	mov     dl, byte [dbr_device]
	mov     ah, 0x42
    mov     si, DAP
	int     0x13
	jnc     load_success
    cmp     ah, 0x80 ; time out ?
    je      load_error
    dec     di
    jl      load_error
    xor     ah, ah
    int     0x13
    jnc     load_boot_block
    jmp     load_error

load_success:
    jmp     0 : BOOT_BLOCK_OFFSET

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
    db  "Load Boot Block Error: "
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

DAP:  
    PacketSize      db  0x10    ; this always is 16 bytes
    Reserved        db  0x0
    BlockCount      dw  0x0
    BufferOffset    dw  0x0
    BufferSegment   dw  0x0
    BlockNumLow     dd  0x0
    BlockNumHigh    dd  0x0

fill:
    times   0x1fe - ($ - $$)    db  0x0

magic:
    dw  0xaa55

