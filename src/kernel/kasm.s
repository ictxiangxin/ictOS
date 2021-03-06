;==============================================================================;
;                             ictOS Asm Functions                              ;
;                                                                      by: ict ;
;==============================================================================;

%include "sysaddr.inc"
%include "protect.inc"
%include "../io/VGAinfo.inc"
%include "../io/BIOScolor.inc"
%include "../io/i8259a.inc"

EBP_FIRST_ARG   equ     0x8
ESP_FIRST_ARG   equ     0x4
NEXT_ARG        equ     0x4

DEFAULT_COLOR   equ     color_white

SCREEN_HIGH     equ     0x19
SCREEN_WIDTH    equ     0x50

CLOCK_INT   equ     0x20

global ict_out
global ict_outs
global ict_in
global ict_ins
global ict_cli
global ict_sti
global ict_lock
global ict_unlock
global ict_memcpy
global ict_clear
global ict_cursorlocation
global ict_setcursor
global ict_startlocation
global ict_setstart
global ict_putc
global ict_refreshvideo
global ict_setupint
global ict_loadLDT
global ict_loadGD
global ict_execute
global ict_done

[bits 32]
;==============================================================================;
; send a byte to port                                                          ;
; VOID ict_out(DWORD port, BYTE data);                                         ;
;==============================================================================;
ict_out:
    mov     edx, dword [esp + ESP_FIRST_ARG] ; edx = port
    mov     al, byte [esp + ESP_FIRST_ARG + NEXT_ARG] ; al = data
    out     dx, al  ; send the data to port
    nop ; delay to wait this I/O operation
    ret ; function end

;==============================================================================;
; translate data word by word from buff to port                                ;
; BYTE ict_outs(DWORD port, BYTE* buf, DWORD size);                            ;
;==============================================================================;
ict_outs:
    push    ebp
    mov     ebp, esp
    push    esi
    mov     edx, dword [ebp + EBP_FIRST_ARG] ; edx = port
    mov     esi, dword [ebp + EBP_FIRST_ARG + NEXT_ARG] ; edi = buff
    mov     ecx, dword [ebp + EBP_FIRST_ARG + NEXT_ARG + NEXT_ARG] ; ecx = size
    shr     ecx, 1 ; the size by word
    cld ; clear the direction flag
    rep outsw ; word by word
    nop ; delay to wait this I/O operation
    pop     esi
    pop     ebp
    ret ; function end

;==============================================================================;
; receive a byte from port                                                     ;
; BYTE ict_in(DWORD port);                                                     ;
;==============================================================================;
ict_in:
    mov     edx, dword [esp + ESP_FIRST_ARG] ; edx = port
    xor     eax, eax ; clear the eax, for return
    in      al, dx ; receive the data from port
    nop ; delay to wait this I/O operation
    ret ; return al

;==============================================================================;
; translate data word by word from port to buff                                ;
; BYTE ict_ins(DWORD port, BYTE* buf, DWORD size);                             ;
;==============================================================================;
ict_ins:
    push    ebp
    mov     ebp, esp
    push    edi
    mov     edx, dword [ebp + EBP_FIRST_ARG] ; edx = port
    mov     edi, dword [ebp + EBP_FIRST_ARG + NEXT_ARG] ; edi = buff
    mov     ecx, dword [ebp + EBP_FIRST_ARG + NEXT_ARG + NEXT_ARG] ; ecx = size
    shr     ecx, 1 ; the size by word
    cld     ; clear the direction flag
    rep insw ; word by word
    nop ; delay to wait this I/O operation
    pop     edi
    pop     ebp
    ret ; function end

;==============================================================================;
; clear interrupt flag                                                         ;
; VOID ict_cli();                                                              ;
;==============================================================================;
ict_cli:
    cli ; clear interrupt flag
    ret ; function end

;==============================================================================;
; set interrupt flag                                                           ;
; VOID ict_sti();                                                              ;
;==============================================================================;
ict_sti:
    sti ; set interrupt flag
    ret ; function end

;==============================================================================;
; test and lock                                                                ;
; DWORD ict_lock(DWORD* lock);                                                 ;
;==============================================================================;
ict_lock:
    mov eax, 0x1
    mov edx, dword [esp + ESP_FIRST_ARG]
    xchg al, byte [edx]
    ret ; function end

;==============================================================================;
; unlock                                                                       ;
; VOID ict_unlock(DWORD* lock);                                                ;
;==============================================================================;
ict_unlock:
    mov edx, dword [esp + ESP_FIRST_ARG]
    mov byte [edx], 0x0
    ret ; function end

;==============================================================================;
; copy a memory block to destination                                           ;
; VOID ict_memcpy(POINTER source, POINTER destination, DWORD size);            ;
;==============================================================================;
ict_memcpy:
    push    ebp
    mov     ebp, esp
    push    edi
    push    esi
    mov     esi, dword [ebp + EBP_FIRST_ARG] ; esi = source
    mov     edi, dword [ebp + EBP_FIRST_ARG + NEXT_ARG] ; edi = destination
    mov     ecx, dword [ebp + EBP_FIRST_ARG + NEXT_ARG + NEXT_ARG] ; ecx = size
    cld ; clear the direction flag, for next copy
    rep movsb ; copy the data byte by byte
    pop     esi
    pop     edi
    pop     ebp
    ret ; function end

;==============================================================================;
; clear a block of memory                                                      ;
; VOID ict_clear(POINTER mem, DWORD size);                                     ;
;==============================================================================;
ict_clear:
    push    ebp
    mov     ebp, esp
    push    edi
    mov     edi, dword [ebp + EBP_FIRST_ARG]
    xor     eax, eax
    mov     ecx, dword [ebp + EBP_FIRST_ARG + NEXT_ARG]
    cld
    rep stosb
    pop     edi
    pop     ebp
    ret

;==============================================================================;
; get the cursor location                                                      ;
; DWORD ict_cursorlocation()                                                   ;
;==============================================================================;
ict_cursorlocation:
    xor     ecx, ecx
    mov     dx, CRTCR_ADDR ; port is the CRT addr reg
    mov     al, CURSOR_HIGH ; we want to get the high part of cursor location
    out     dx, al ; send the msg to port
    nop ; delay
    mov     dx, CRTCR_DATA ; port is the CRT data reg
    in      al, dx ; receive the high part of cursor location
    nop ; delay
    mov     ch, al ; store the high part of cursor location to ch
    mov     dx, CRTCR_ADDR ; port is the CRT addr reg
    mov     al, CURSOR_LOW ; we want to get the low part of cursor location
    out     dx, al ; send the msg to port
    nop ; delay
    mov     dx, CRTCR_DATA ; port is the CRT data reg
    in      al, dx ; receive the low part of cursor location
    nop ; delay
    mov     cl, al ; store the low part of cursor location to cl
    mov     eax, ecx
    ret

;==============================================================================;
; set the cursor location                                                      ;
; VOID ict_setcursor(DWORD loc)                                                ;
;==============================================================================;
ict_setcursor:
    mov     ecx, dword [esp + ESP_FIRST_ARG]
    mov     dx, CRTCR_ADDR ; port is the CRT addr reg
    mov     al, CURSOR_LOW ; we want to set the low part of the cursor location
    out     dx, al ; send the msg to port
    nop ; delay
    mov     al, cl ; load the low part to al
    mov     dx, CRTCR_DATA  ; set the port to data
    out     dx, al ; send the low part
    nop ; delay
    mov     dx, CRTCR_ADDR ; port is the CRT addr reg
    mov     al, CURSOR_HIGH ; we want to set the high part of the cursor location
    out     dx, al ; send the msg to port
    nop ; delay
    mov     al, ch ; load the high part to al
    mov     dx, CRTCR_DATA  ; set the port to data
    out     dx, al ; send the high part
    nop ; delay
    ret

;==============================================================================;
; get the video start location                                                 ;
; DWORD ict_startlocation()                                                    ;
;==============================================================================;
ict_startlocation:
    xor     ecx, ecx ; ebx = 0
    mov     dx, CRTCR_ADDR ; port is the CRT addr reg
    mov     al, START_LOW ; we want to get the low part of start location
    out     dx, al ; send the msg to port
    nop ; delay
    mov     dx, CRTCR_DATA  ; port is the CRT data reg
    in      al, dx ; receive the low part of start location
    nop ; delay
    mov     cl, al ; store low part of start location to bl
    mov     dx, CRTCR_ADDR  ; port is the CRT addr reg
    mov     al, START_HIGH  ; we want to get the high part of start location
    out     dx, al ; send the msg to port
    nop ; delay
    mov     dx, CRTCR_DATA  ; port is the CRT data reg
    in      al, dx ; receive the high part of start location
    nop ; delay
    mov     ch, al ; store high part of start location to bh
    mov     eax,ecx
    ret

;==============================================================================;
; set the video start location                                                 ;
; VOID ict_setstart(DWORD loc)                                                 ;
;==============================================================================;
ict_setstart:
    mov     ecx, dword [esp + ESP_FIRST_ARG]
    mov     dx, CRTCR_ADDR ; port is the CRT addr reg
    mov     al, START_LOW ; we want to set the low part of the start location
    out     dx, al ; send the msg to port
    nop ; delay
    mov     al, cl ; load the low part to al
    mov     dx, CRTCR_DATA  ; set the port to data
    out     dx, al ; send the low part
    nop ; delay
    mov     dx, CRTCR_ADDR ; port is the CRT addr reg
    mov     al, START_HIGH ; we want to set the high part of the start location
    out     dx, al ; send the msg to port
    nop ; delay
    mov     al, ch ; load the high part to al
    mov     dx, CRTCR_DATA  ; set the port to data
    out     dx, al ; send the high part
    nop ; delay
    ret

;==============================================================================;
; write graphic memory to print one char                                       ;
; VOID ict_putchar(DWORD c)                                                    ;
;==============================================================================;
ict_putc:
    mov     bx, word [esp + ESP_FIRST_ARG]
    mov     edx, dword [esp + ESP_FIRST_ARG + NEXT_ARG]
    shl     edx, 0x1
    mov     word [gs:edx], bx
    ret

;==============================================================================;
; refresh video setting                                                        ;
; VOID ict_refreshvideo(DWORD start, DWORD width, DWORD high, DWORD buffsize)  ;
;==============================================================================;
ict_refreshvideo:
    push    ebp
    mov     ebp, esp
    push    edi
    push    esi
    mov     esi, dword [ebp + EBP_FIRST_ARG]
    shl     esi, 0x1
    add     esi, VIDEO_LOCATION
    mov     edi, VIDEO_LOCATION
    mov     eax, dword [ebp + EBP_FIRST_ARG + NEXT_ARG]
    mov     ebx, dword [ebp + EBP_FIRST_ARG + NEXT_ARG + NEXT_ARG]
    mul     ebx
    mov     ecx, eax
    mov     ebx, eax
    cld
    rep movsw
    mov     al, 0x20 ; use space to fill
    mov     ah, DEFAULT_COLOR ; set the color
    mov     ecx, dword [ebp + EBP_FIRST_ARG + NEXT_ARG + NEXT_ARG + NEXT_ARG]
    sub     ecx, ebx
    mov     edi, VIDEO_LOCATION
    shl     ebx, 0x1
    add     edi, ebx
    rep stosw
    pop     esi
    pop     edi
    pop     ebp
    ret

;==============================================================================;
; setup a interrupt to IDT                                                     ;
; VOID ict_setupint(DWORD int_number, POINTER int_func);                       ;
;==============================================================================;
ict_setupint:
    push    ebp
    mov     ebp, esp
    push    edi
    mov     eax, dword [ebp + EBP_FIRST_ARG] ; eax = int_number
    shl     eax, 0x3 ; eax *= 8, for each int desc has 8 bytes
    add     eax, IDT_LOCATION ; compute the abs addr of idt
    mov     edi, eax ; edi = abs addr of idt
    mov     word [edi + 0x4], DAT_80386igate ; set the attr
    mov     ax, cs ; ax = cs
    mov     word [edi + 0x2], ax ; set the selector
    mov     eax, dword [ebp + EBP_FIRST_ARG + NEXT_ARG] ; eax = addr of int_func
    mov     word [edi], ax ; set the offset[0..15]
    mov     ecx, 0x10 ; ecx = 16
    shr     eax, cl ; ax = offset[16..31]
    mov     word [edi + 0x6], ax ; set the offset[16..31]
    pop     edi
    pop     ebp
    ret ; function end

;==============================================================================;
; load the LDT to memory and set the ldtr                                      ;
; VOID ict_loadLDT(DWORD GD_number);                                           ;
;==============================================================================;
ict_loadLDT:
    mov     eax, dword [esp + ESP_FIRST_ARG] ; eax = GD_number
    shl     eax, 0x3 ; computer the addr, for each GD has 8 bytes
    lldt    ax ; load the GD as ldt desc
    ret ; function end

;==============================================================================;
; load the GDT to memory                                                       ;
; VOID ict_loadGD(SEGDESC* gd_temp, DWORD number);                             ;
;==============================================================================;
ict_loadGD:
    push    ebp
    mov     ebp, esp
    push    edi
    push    esi
    mov     eax, dword [ebp + EBP_FIRST_ARG + NEXT_ARG] ; eax = number
    shl     eax, 0x3 ; compute the addr, for each desc has 8 bytes
    mov     edi, eax ; edi = addr of desc
    mov     esi, dword [ebp + EBP_FIRST_ARG] ; esi = addr of temp gd
    mov     ecx, 0x8 ; each gd has 8 bytes
    cld ; clear the direction flag, for next copy
    rep movsb ; copy the temp gd to gdt
    pop     esi
    pop     edi
    pop     ebp
    ret ; function end

;==============================================================================;
; execute the process                                                          ;
; VOID ict_execut(kPROC* proc);                                                ;
;==============================================================================;
ict_execute:
    cli ; close the interrupt
    mov     esi, dword [esp + ESP_FIRST_ARG]
    mov     esp, dword [esi + 0x3c] ; esp is at offset 60
    sub     esp, 0x44 ; alloc 68 bytes to regs list
    mov     edi, esp ; for next movsb
    mov     ecx, 0x44 ; regs list has 68 bytes
    cld ; make sure the destination is right
    rep movsb ; copy the regs list
    pop     fs ; -.
    pop     gs ;  | restore the segment regs
    pop     es ;  |
    pop     ds ; -'
    popa ; restore all general regs
    iretd ; load the eip, cs, and eflags. so the proc is execute

;==============================================================================;
; kproc's works done, it must change to another kproc                          ;
; VOID ict_done();                                                             ;
;==============================================================================;
ict_done:
    int     CLOCK_INT
    ret
