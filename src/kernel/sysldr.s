;==============================================================================;
;                            ictOS System Loader                               ;
;                                                                      by: ict ;
;==============================================================================;

%include "../io/BIOScolor.inc"
%include "../io/VGAinfo.inc"
%include "../io/i8259a.inc"
%include "sysaddr.inc"
%include "sysldrmacro.inc"
%include "sysldr.inc"
%include "protect.inc"

[section real_mode]
[bits 16]

sysldr:
real_start:
    call    real_get_ram_info
    jmp     into_protected_mode

;==============================================================================;
; get basic information about this computer                                    ;
;==============================================================================;
real_get_ram_info:
    xor     ebx, ebx    ; ebx = 0, for first int 0x15
next_ARDS:
    mov     di, ARDS    ; the offset of ARDS need load to di
    mov     ecx, 0x14   ; the size of the ARDS
    mov     edx, 0x534d4150 ; the magic number of the int 0x15
    mov     eax, 0xe820 ; the function number of int 0x15 to get memory info
    int     0x15    ; use BIOS int 0x15
    jc      error_ARDS  ; if cf was setted, the int 0x15 is fault
    mov     eax, dword [di] ; load the base of memory block to eax
    add     di, 0x8 ; di point to the offset of memory block
    add     eax, dword [di] ; add the offset of memory block to eax
    cmp     eax, dword [MemorySize] ; if the memory size is bigger than front
    jb      do_not_record   ; if smaller, the do not record this value
    mov     dword [MemorySize], eax ; bigger, then record this as new memory size
do_not_record:
    test    ebx, ebx    ;if ebx == 0, then the memory info is end
    jnz     next_ARDS   ;if ebx != 0, then read the next ARDS
    ret     ; the function end
error_ARDS:
    mov     ax, cs  ; let ax == cs
    mov     es, ax  ; let es == cs, for next print use es
    mov     bp, MemoryCheckError    ; bp == offset of NoStaterMsg
    mov     bl, color_red   ; set color
    xor     ah, ah  ; set function number(print string)
    int     0x20    ; use sys call to print
    int     0x23    ; sysldr end
MemoryCheckError    db  "ERROR: Memory Check fail", 0x0

ARDS    times   0x14    db  0x0 ; 20bits ARDS
MemorySize  dd  0x0

into_protected_mode:
    LoadDesc    kernel_desc,           0x0,         MEMORY_LOCATION,  MEMORY_LIMIT,                 DAT_KERNEL
    LoadDesc    memory_desc,           0x0,         MEMORY_LOCATION,  MEMORY_LIMIT,                 DAT_MEMORY
    LoadDesc    stack_seg_desc,        0x0,         STACK_LOCATION,   STACK_LIMIT,                  DAT_STACK_SEG
    LoadDesc    video_seg_desc,        0x0,         VIDEO_LOCATION,   VIDEO_LIMIT,                  DAT_VIDEO_SEG
    LoadDesc    exception_report_desc, 0x0,         EXCEPT_LOCATION,  EXCEPT_LIMIT,                 DAT_EXCEPT
    LoadDesc    exception_exec_desc,   0x0,         EXCEPT_LOCATION,  EXCEPT_LIMIT,                 DAT_EXCEPT_EXEC
    LoadDesc    tss_desc,              cs,          TSS,              TSS_LEN - 1,                  DAT_TSS
    LoadDesc    protected_sysldr_desc, cs,          protected_sysldr, protected_sysldr_lenght - 1,  DAT_PROTECTED_SYSLDR
    LoadDesc    data_seg_desc,         cs,          sysldr,           DATA_LIMIT,                   DAT_DATA_SEG
    LoadDesc    temp_GDT_desc,         cs,          GDT,              GDT_lenght + GDTR_length- 1 , DAT_TEMP_GDT
    LoadDesc    real_GDT_desc,         0x0,         GDT_LOCATION,     GDT_LIMIT,                    DAT_REAL_GDT
    LoadDesc    IDT_desc,              0x0,         IDT_LOCATION,     IDT_LIMIT,                    DAT_IDT
    LoadDesc    PG_desc,               0x0,         PG_LOCATION,      PG_LIMIT,                     DAT_PG
    LoadDesc    kernel_file_desc,      KERNEL_BASE, KERNEL_OFFSET,    KERNEL_LIMIT,                 DAT_KERNEL_FILE
    xor     eax, eax    ; eax = 0
    mov     ax, ds  ; ax(eax) = ds
    shl     eax, 0x4    ; ax(eax) *= 16
    add     eax, GDT    ; eax += GDT address
    mov     dword [GDTRBase], eax   ; save GDT base into variable
    lgdt    [GDTR]  ; load GDT information into GDTR
    cli ; clear intrrupt flag, before we reset the intrrupt mechanism in protected mode
    in      al, 92h         ; -.
    or      al, 00000010b   ;  | open A20, so that we can use more memory
    out     92h, al         ; -'
    mov     eax, cr0    ; -.
    or      eax, 1      ;  | set PE = 1 of cr0, to open protected mode
    mov     cr0, eax    ; -'
    jmp     dword protected_sysldr_selector : 0 ; use jmp to load selector, and here is 16bit code to 32bit code

;==============================================================================;
; loader GDT into right structure                                              ;
; si = address of the segment information                                      ;
; di = address of the gdt                                                      ;
;                                                                              ;
; segment information:                                                         ;
;     SegLimit - |byte3|byte2|byte1|byte0| {dword}                             ;
;     SegBase -  |byte3|byte2|byte1|byte0| {dword}                             ;
;     SegAttr -  |byte1|byte0| {word}                                          ;
;                                                                              ;
; | byte7 | byte6 | byte5 | byte4 | byte3 | byte2 | byte1 | byte0 |            ;
; | 31..24|  Attribution  |         23..0         |     15..0     |            ;
; |SegBase|               |        SegBase        |    SegLimit   |            ;
;        /                 \                                                   ;
; |-------------byte6-------------|-------------byte5-------------|            ;
; | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |            ;
; | G |D/B| 0 |AVL|SegLimit 19..16| P |  DPL  | S |     TYPE      |            ;
;==============================================================================;
load_gdt:
    push    ax  ; -. save ax, bx, which will be used next
    push    bx  ; -'
    mov     ax, word [si + 0x0] ; ax = SegLimit(byte0-byte1)
    mov     word [di + 0x0], ax ; load ax to byte0-byte1 of gdt
    mov     ax, word [si + 0x4] ; ax = SegBase(byte0-byte1)
    mov     word [di + 0x2], ax ; load ax to byte2-byte3 of gdt
    mov     al, byte [si + 0x6] ; al = SegBase(byte2)
    mov     byte [di + 0x4], al ; load al to byte4 of gdt
    mov     al, byte [si + 0x7] ; al = SegBase(byte3)
    mov     byte [di + 0x7], al ; load al to byte7 of gdt
    mov     ax, word [si + 0x8] ; ax = SegAttr
    mov     bx, word [si + 0x2] ; bx = SegLimit(byte2-byte3)
    and     bx, 0000000000001111b   ; only 4 bits are segment limit
    shl     bx, 0x8 ; move segment limit into right location
    or      ax, bx  ; combine segment limit and segment attribution
    mov     word [di + 0x5], ax ; load ax to byte5-byte6 of gdt
    pop     bx  ; -. restore ax, bx
    pop     ax  ; -'
    ret ; function end and return

;==============================================================================;
; TSS structure                                                                ;
;==============================================================================;
TSS:
tss_back    dd  0
tss_esp0    dd  STACK_LOCATION + STACK_LIMIT
tss_ss0     dd  memory_selector
tss_esp1    dd  0
tss_ss1     dd  0
tss_esp2    dd  0
tss_ss2     dd  0
tss_cr3     dd  0
tss_eip     dd  0
tss_eflags  dd  0
tss_eax     dd  0
tss_ecx     dd  0
tss_edx     dd  0
tss_ebx     dd  0
tss_esp     dd  0
tss_ebp     dd  0
tss_esi     dd  0
tss_edi     dd  0
tss_es      dd  0
tss_cs      dd  0
tss_ss      dd  0
tss_ds      dd  0
tss_fs      dd  0
tss_gs      dd  0
tss_ldt     dd  0
tss_trap    dw  0
tss_iomap   dw  $ - TSS + 2
tss_ioend   db  0xff
TSS_LEN equ $ - TSS

;==============================================================================;
; GDT information                                                              ;
;==============================================================================;
GDT:
; following DTs must reserved whenever
null_desc               times   0x8 db  0x0 ; reserved by cpu
kernel_desc             times   0x8 db  0x0 ; kernel code DT
memory_desc             times   0x8 db  0x0 ; kernel data DT
stack_seg_desc          times   0x8 db  0x0 ; kernel stack DT
video_seg_desc          times   0x8 db  0x0 ; kernel video DT
exception_report_desc   times   0x8 db  0x0 ; exception report data DT
exception_exec_desc     times   0x8 db  0x0 ; exception report code DT
tss_desc                times   0x8 db  0x0
; following DTs can be covered at kernel executing
protected_sysldr_desc   times   0x8 db  0x0
data_seg_desc           times   0x8 db  0x0
temp_GDT_desc           times   0x8 db  0x0
real_GDT_desc           times   0x8 db  0x0
IDT_desc                times   0x8 db  0x0
PG_desc                 times   0x8 db  0x0
kernel_file_desc        times   0x8 db  0x0
GDT_lenght  equ $ - GDT

GDTR:
GDTRLimit dw GDT_lenght
GDTRBase  dd 0x0

kernel_selector             equ kernel_desc - GDT
memory_selector             equ memory_desc - GDT
stack_seg_selector          equ stack_seg_desc - GDT
video_seg_selector          equ video_seg_desc - GDT
exception_report_selector   equ exception_report_desc - GDT
exception_exec_selector     equ exception_exec_desc - GDT
tss_selector                equ tss_desc - GDT
protected_sysldr_selector   equ protected_sysldr_desc - GDT
data_seg_selector           equ data_seg_desc - GDT
temp_GDT_selector           equ temp_GDT_desc - GDT
real_GDT_selector           equ real_GDT_desc - GDT
IDT_selector                equ IDT_desc - GDT
PG_selector                 equ PG_desc - GDT
kernel_file_selector        equ kernel_file_desc - GDT

;==============================================================================;
; temp variables                                                               ;
;==============================================================================;
TempSegDesc:
SegLimit dd 0x0
SegBase  dd 0x0
SegAttr  dw 0x0

;==============================================================================;
; sysldr in protected mode and has 32bit code and 32bit address                ;
; it will do:                                                                  ;
; 1. move the GDT to address 0x0                                               ;
;==============================================================================;
[section protected_mode]
[bits 32]
protected_sysldr:
; init all segment regs so that sysldr can execute right
init:
    mov  ax, stack_seg_selector ; -. init ss segment
    mov  ss, ax                 ; -'
    mov  esp, STACK_LIMIT ; init sp segment
    mov  ax, data_seg_selector ; -.
    mov  ds, ax                ;  | init ds, es segment
    mov  es, ax                ; -'
    mov  ax, video_seg_selector ; -. init gs segment
    mov  gs, ax                 ; -'
    xor  eax, eax ; clear eax, for next clear eflags
    push eax ; eax to stack
    popf ; clear eflags
main:
    call print
    db color_yellow, ">>> Load Kernel <<<", 0xa, 0x0
    Initialize reload_GDT,       "Reloading GDT ...               "
    Initialize init_8259a,       "Initializing 8259a ...          "
    Initialize init_interrupt,   "Initializing Interrupt ...      "
    Initialize start_page,       "Starting Page Mechanism ...     "
    Initialize load_init_kernel, "Initializing Kernel ...         "
    mov  ax, tss_selector
    ltr  ax
    call print
    db color_yellow, ">>> Start Kernel <<<", 0xa, 0x0
    cli
    mov eax, STACK_LOCATION + STACK_LIMIT ; -. it is important to use flat stack !!!
    mov esp, eax                          ; -'
    mov ax, memory_selector ; -.
    mov ds, ax              ;  |
    mov ss, ax              ;  | it is important to set the flat segment!!!
    mov es, ax              ;  |
    mov fs, ax              ; -'
    jmp kernel_selector : KERNEL_LOCATION

;==============================================================================;
; print, temp screen output                                                    ;
;==============================================================================;
print:
    pop     esi ; pop eip to esi
    add     esi, protected_sysldr   ; let esi base on ds
    cld ; clear direction flag
    lodsb   ; get the color
    mov     ch, al  ; set the color
print_next:
    lodsb   ; load a char to al, esi++
    test    al, al  ; if al == NULL
    jz      print_done  ; string is end
    cmp     al, 0xa ; if the char is RETURN
    je      print_newline   ;print a new line
    mov     cl, al  ; save char to cl
    xor     eax, eax    ; eax = 0
    mov     al, byte [Print_x]  ; get the x record
    mov     bl, 0x50            ; -.
    mul     bl                  ;  |
    xor     bx, bx              ;  | (x, y) = ( x * 80 + y ) * 2
    mov     bl, byte [Print_y]  ;  |
    add     ax, bx              ;  |
    shl     ax, 0x1             ; -'
    mov     edi, eax    ; edi = the address of the char in video memory
    mov     word [gs:edi], cx   ; write the char into video memory
    mov     al, byte [Print_y]  ; -. y++
    inc     al                  ; -'
    mov     byte [Print_y], al  ; save y
    cmp     al, 0x50    ; if y == 0x50, means one line end
    jne     print_next  ; print next char
print_newline:
    xor     al, al              ; -. y = 0
    mov     byte [Print_y], al  ; -'
    mov     al, byte [Print_x]  ; -. x++
    inc     al                  ; -'
    mov     byte [Print_x], al  ; save x
    jmp     print_next  ; print next char
print_done:
    mov     al, byte [Print_x]  ; get Print_x to al
    mov     bl, 0x50    ; bl = 80, for one line has 80 chars
    mul     bl  ; ax point to the line head
    xor     bx, bx  ; bx = 0, for next we only use bl
    mov     bl, byte [Print_y]  ; get Print_y to bl
    add     ax, bx  ; ax = the right location(offset)
    mov     bx, ax  ; save the location to bx
    PortOut CRTCR_ADDR, CURSOR_LOW  ; tell CRT, we will load low address of cursor location
    PortOut CRTCR_DATA, bl  ; send the low address of cursor location
    PortOut CRTCR_ADDR, CURSOR_HIGH ; tell CRT, we will load high address of cursor location
    PortOut CRTCR_DATA, bh  ; send the hogh address of cursor location
    sub     esi, protected_sysldr   ; let esi base on cs
    jmp     esi ; now esi point to the data after the string

Print_x db  0x2
Print_y db  0x0

;==============================================================================;
; reload GDT to right location                                                 ;
;==============================================================================;
reload_GDT:
    push    ds  ; -. save ds, es
    push    es  ; -'
    mov     ax, temp_GDT_selector   ; -. load the old GDT base to ds
    mov     ds, ax                  ; -'
    mov     ax, real_GDT_selector   ; -. load the new GDT base to es
    mov     es, ax                  ; -'
    xor     esi, esi    ; esi = 0
    xor     edi, edi    ; edi = 0
    cld ; clear direction flag
    mov     ecx, GDT_lenght ; GDT size and it is the move size
    rep movsb   ; move the GDT to new location
    mov     esi, GDT_lenght ; esi = GDT size, it points to the GDTR struct
    mov     word [esi], GDT_LIMIT   ; set the new GDT limit
    mov     dword [esi + 0x2], GDT_LOCATION ; set the new GDT address
    lgdt    [esi]   ; load new GDTR and it points to the new GDT
    pop     es  ; -. restore ds, es
    pop     ds  ; -'
    ret

;==============================================================================;
; init 8259a                                                                   ;
;==============================================================================;
init_8259a:
    PortOut MASTER_8259A_ICW1, 00010001b ; ICW1 to master 8259a, need ICW4
    PortOut SLAVE_8259A_ICW1,  00010001b ; ICW1 to slave 8259a, need ICW4
    PortOut MASTER_8259A,      00100000b ; ICW2 to master 8259a, int 0x20 ~ 0x27 mapping IRQ0 ~ IRQ7
    PortOut SLAVE_8259A,       00101000b ; ICW2 to slave 8259a, int 0x28 ~ 0x2f mapping IRQ8 ~ IRQ15
    PortOut MASTER_8259A,      00000100b ; ICW3 to master 8259a, master 8259a IRQ2 link slave 8259a
    PortOut SLAVE_8259A,       00000010b ; ICW3 to slave 8259a, link to master 8259a IRQ2
    PortOut MASTER_8259A,      00000011b ; ICW4 to master 8259a, 80x86 mode and auto EOI
    PortOut SLAVE_8259A,       00000011b ; ICW4 to slave 8259a, 80x86 mode and auto EOI
    PortOut MASTER_8259A,      11111111b ; mask all interrupts of master 8259a
    PortOut SLAVE_8259A,       11111111b ; mask all interrupts of slave 8259a
    ret ; function end

;==============================================================================;
; Init IDT and load all exception interrupt and open interrupt mechanism       ;
;==============================================================================;
init_interrupt:
    push    es  ; save es
    mov     ax, IDT_selector    ; ax = IDT_selector
    mov     es, ax  ; load the IDT_selector to es
    xor     eax, eax    ; clear eax, for next we will clear a block of memory use eax
    xor     edi, edi    ; edi = 0, point to the start of the IDT
    mov     ecx, IDT_LIMIT / 0x8 + 0x1  ; compute the IDT elem sum, load to ecx, for next rep
    cld ; clear the direction flag, for next stosd
    rep stosd   ; load eax to [es:edi], to clear this block memory
    LoadInt 0x00, EXCEPT_DE   - exception_report, exception_exec_selector, DAT_INTERRUPT    ; -.
    LoadInt 0x01, EXCEPT_DB   - exception_report, exception_exec_selector, DAT_INTERRUPT    ;  |
    LoadInt 0x02, EXCEPT_NMI  - exception_report, exception_exec_selector, DAT_INTERRUPT    ;  |
    LoadInt 0x03, EXCEPT_INT3 - exception_report, exception_exec_selector, DAT_INTERRUPT    ;  |
    LoadInt 0x04, EXCEPT_OF   - exception_report, exception_exec_selector, DAT_INTERRUPT    ;  | load all exceptions desc to IDT
    LoadInt 0x05, EXCEPT_BR   - exception_report, exception_exec_selector, DAT_INTERRUPT    ;  | now, our OS can report the exceptions
    LoadInt 0x06, EXCEPT_UD   - exception_report, exception_exec_selector, DAT_INTERRUPT    ;  | if our OS is crash
    LoadInt 0x07, EXCEPT_NM   - exception_report, exception_exec_selector, DAT_INTERRUPT    ;  | we can see the "blue screen"
    LoadInt 0x08, EXCEPT_DF   - exception_report, exception_exec_selector, DAT_INTERRUPT    ;  | and get may information about this crash
    LoadInt 0x09, EXCEPT_CBR  - exception_report, exception_exec_selector, DAT_INTERRUPT    ;  |
    LoadInt 0x0a, EXCEPT_TS   - exception_report, exception_exec_selector, DAT_INTERRUPT    ;  |
    LoadInt 0x0b, EXCEPT_NP   - exception_report, exception_exec_selector, DAT_INTERRUPT    ;  |
    LoadInt 0x0c, EXCEPT_SS   - exception_report, exception_exec_selector, DAT_INTERRUPT    ;  |
    LoadInt 0x0d, EXCEPT_GP   - exception_report, exception_exec_selector, DAT_INTERRUPT    ;  |
    LoadInt 0x0e, EXCEPT_PF   - exception_report, exception_exec_selector, DAT_INTERRUPT    ;  |
    LoadInt 0x0f, EXCEPT_RS   - exception_report, exception_exec_selector, DAT_INTERRUPT    ;  |
    LoadInt 0x10, EXCEPT_MF   - exception_report, exception_exec_selector, DAT_INTERRUPT    ;  |
    LoadInt 0x11, EXCEPT_AC   - exception_report, exception_exec_selector, DAT_INTERRUPT    ;  |
    LoadInt 0x12, EXCEPT_MC   - exception_report, exception_exec_selector, DAT_INTERRUPT    ;  |
    LoadInt 0x13, EXCEPT_XF   - exception_report, exception_exec_selector, DAT_INTERRUPT    ; -'
    mov     ax, exception_report_selector   ; load exception report programme selector to ax
    mov     es, ax  ; load the selector to es
    xor     edi, edi    ; edi = 0
    mov     esi, exception_report   ; load the offset of exception programme to esi
    mov     ecx, exception_report_length    ; load the lenght of the exception programme to ecx, for next rep
    rep movsb   ; move the exception programme to right place
    lidt    [IDTR]  ; load the IDR and we can report all exceptions
    pop     es  ; restore es
    ret ; function end

temp_IDesc  times   0x8 db  0x0 ; the temp Interrupt Description area

IDTR:
IDTRLimit   dw  IDT_LIMIT
IDTRBase    dd  IDT_LOCATION

;==============================================================================;
; temp variables                                                               ;
;==============================================================================;
TempIntDesc:
IntOffset   dd 0x0
IntSelector dw 0x0
IntAttr     dw 0x0

;==============================================================================;
; load the interrupt descriptor to IDT                                         ;
;==============================================================================;
load_idt:
    push    es  ; -. save es and ds
    push    ds  ; -'
    mov     ax, data_seg_selector   ; load the data selector to ax
    mov     ds, ax  ; -. make the ds and es all loaded data selector
    mov     es, ax  ; -'
    mov     eax, dword [esi]    ; eax = offset
    mov     word [edi], ax  ; load the low address of offset
    shr     eax, 0x8    ; make the high address of offset to ax
    mov     word [edi + 0x6], ax    ; load high address of off set
    mov     ax, word [esi + 0x4]    ; ax = selector
    mov     word [edi + 0x2], ax    ; load the selector
    mov     ax, word [esi + 0x6]    ; ax = attr
    mov     word [edi + 0x4], ax    ; load the attr
    pop     ds  ; -. restore the ds and es
    pop     es  ; -'
    ret ; function end

;==============================================================================;
; report the critical exception and halt the computer                          ;
; for this function will move to a new location, so we write these code mush   ;
; use offset about the start of this function: exception_report                ;
;==============================================================================;
exception_report:
EXCEPT_DE:
    push    0x0 ; this exception has not error code, and we set is zero
    mov     eax, 0x0    ; set the vector number to al(eax)
    jmp     print_exception_error   ; goto main exception function
EXCEPT_DB:
    push    0x0
    mov     eax, 0x1
    jmp     print_exception_error
EXCEPT_NMI:
    push    0x0
    mov     eax, 0x2
    jmp     print_exception_error
EXCEPT_INT3:
    push    0x0
    mov     eax, 0x3
    jmp     print_exception_error
EXCEPT_OF:
    push    0x0
    mov     eax, 0x4
    jmp     print_exception_error
EXCEPT_BR:
    push    0x0
    mov     eax, 0x5
    jmp     print_exception_error
EXCEPT_UD:
    push    0x0
    mov     eax, 0x6
    jmp print_exception_error
EXCEPT_NM:
    push    0x0
    mov     eax, 0x7
    jmp     print_exception_error
EXCEPT_DF:  ; here, this kind of exception has its error code, and we needn't set it
    mov     eax, 0x8
    jmp     print_exception_error
EXCEPT_CBR:
    push    0x0
    mov     eax, 0x9
    jmp     print_exception_error
EXCEPT_TS:
    mov     eax, 0xa
    jmp     print_exception_error
EXCEPT_NP:
    mov     eax, 0xb
    jmp     print_exception_error
EXCEPT_SS:
    mov     eax, 0xc
    jmp     print_exception_error
EXCEPT_GP:
    mov     eax, 0xd
    jmp     print_exception_error
EXCEPT_PF:
    mov     eax, 0xe
    jmp     print_exception_error
EXCEPT_RS:
    push    0x0
    mov     eax, 0xf
    jmp     print_exception_error
EXCEPT_MF:
    push    0x0
    mov     eax, 0x10
    jmp     print_exception_error
EXCEPT_AC:
    mov     eax, 0x11
    jmp     print_exception_error
EXCEPT_MC:
    push    0x0
    mov     eax, 0x12
    jmp     print_exception_error
EXCEPT_XF:
    push    0xc
    mov     eax, 0x13
    jmp     print_exception_error
print_exception_error:  ; here is the exception report funtion
    cli
    push    eax ; save eax, until we load it and save the vector number
    mov     ax, video_seg_selector  ; -. init the video segment register
    mov     gs, ax                  ; -'
    mov     ax, exception_report_selector   ; -.
    mov     ds, ax                          ;  | init the data and stack segment register
    mov     es, ax                          ; -'
    PortOut CRTCR_ADDR, START_HIGH  ; tell the CRT we will set the high address of vdieo start location
    PortOut CRTCR_DATA, 0x0 ; set the low address of video start location
    PortOut CRTCR_ADDR, START_LOW   ; tell the CRT we will set the low address of video start location
    PortOut CRTCR_DATA, 0x0 ; set the high address of video start location
    PortOut CRTCR_ADDR, CURSOR_HIGH ; tell the CRT we will set the high address of cursor location
    PortOut CRTCR_DATA, 0xff    ; set the high address of cursor location
    PortOut CRTCR_ADDR, CURSOR_LOW  ; tell the CRT we will set the low address of cursor location
    PortOut CRTCR_DATA, 0xff    ; set the low address of cursor location
    pop     eax ; restore eax
    mov     byte [VectorNumber - exception_report], al  ; save the vector number
    push    edx ; save edx
    mov     edx, 489    ; set the eax print loction
    call    load_32reg  ; load the eax value into CrashMsg
    pop     edx ; restore edx
    pop     dword [ErrorCode - exception_report]    ; get the error code and save it
    pop     dword [SaveEIP - exception_report]  ; get the eip and save it
    pop     dword [SaveCS - exception_report]   ; get the cs and save it
    pop     dword [SaveEFLAGS - exception_report]   ; get the eflags and save it
    mov     ax, stack_seg_selector  ; -.
    mov     ss, ax                  ;  | reset the stack and we use the kernel stack
    mov     esp, STACK_LIMIT        ; -'
    mov     eax, edx    ; -.
    mov     edx, 509    ;  | load the edx value into CrashMsg
    call    load_32reg  ; -'
    mov     eax, ecx    ; -.
    mov     edx, 529    ;  | load the ecx value into CrashMsg
    call    load_32reg  ; -'
    mov     eax, ebx    ; -.
    mov     edx, 549    ;  | load the ebx value into CrashMsg
    call    load_32reg  ; -'
    mov     eax, esp    ; -.
    mov     edx, 569    ;  | load the esp value into CrashMsg
    call    load_32reg  ; -'
    mov     eax, ebp    ; -.
    mov     edx, 589    ;  | load the ebp value into CrashMsg
    call    load_32reg  ; -'
    mov     eax, esi    ; -.
    mov     edx, 609    ;  | load the esi value into CrashMsg
    call    load_32reg  ; -'
    mov     eax, edi    ; -.
    mov     edx, 629    ;  | load the edi value into CrashMsg
    call    load_32reg  ; -'
    mov     ax, es      ; -.
    mov     edx, 728    ;  | load the es value into CrashMsg
    call    load_16reg  ; -'
    mov     ax, ss      ; -.
    mov     edx, 743    ;  | load the ss value into CrashMsg
    call    load_16reg  ; -'
    mov     ax, ds      ; -.
    mov     edx, 758    ;  | load the ds value into CrashMsg
    call    load_16reg  ; -'
    mov     ax, fs      ; -.
    mov     edx, 773    ;  | load the fs value into CrashMsg
    call    load_16reg  ; -'
    mov     ax, gs      ; -.
    mov     edx, 788    ;  | load the gs value into CrashMsg
    call    load_16reg  ; -'
    mov     eax, dword [SaveEFLAGS - exception_report]  ; -.
    mov     edx, 890                                    ;  | load the eflags value into CrashMsg
    call    load_reg_bit                                ; -'
    mov     eax, cr0        ; -.
    mov     edx, 1047       ;  | load the cr0 value into CrashMsg
    call    load_reg_bit    ; -'
    mov     eax, cr2        ; -.
    mov     edx, 1127       ;  | load the cr2 value into CrashMsg
    call    load_reg_bit    ; -'
    mov     eax, cr3        ; -.
    mov     edx, 1207       ;  | load the cr3 value into CrashMsg
    call    load_reg_bit    ; -'
    sgdt    [DTR_struct - exception_report] ; save the GDTR to [DTR_struct]
    mov     eax, dword [DTR_base - exception_report]    ; -.
    mov     edx, 1456                                   ;  | load the gdtr base value into CrashMsg
    call    load_32reg                                  ; -'
    mov     ax, word [DTR_limit - exception_report] ; -.
    mov     edx, 1476                               ;  | load the gdtr limit value into CrashMsg
    call    load_16reg                              ; -'
    sidt    [DTR_struct - exception_report] ; save the IDTR to [DTR_struct]
    mov     eax, dword [DTR_base - exception_report]    ; -.
    mov     edx, 1536                                   ;  | load the idtr base value into CrashMsg
    call    load_32reg                                  ; -'
    mov     ax, word [DTR_limit - exception_report] ; -.
    mov     edx, 1556                               ;  | load the idtr limit value into CrashMsg
    call    load_16reg                              ; -'
    mov     ax, word [SaveCS - exception_report]    ; -.
    mov     edx, 1688                               ;  | load the cs value into CrashMsg
    call    load_16reg                              ; -'
    mov     eax, dword [SaveEIP - exception_report] ; -.
    mov     edx, 1704                               ;  | load the eip value into CrashMsg
    call    load_32reg                              ; -'
    xor     eax, eax    ; clear eax, for next we only use al
    mov     al, byte [VectorNumber - exception_report]  ; al = vector number
    call    load_msg    ; according to the al, we can load the right error msg into CrashMsg
    mov     eax, dword [ErrorCode - exception_report]   ; -.
    mov     edx, 389                                    ;  | load the error code into CrashMsg
    call    load_32reg                                  ; -'
    mov     esi, CrashMsg - exception_report    ; esi point to the CrashMsg
    call    print_crash_msg ; print the CrashMsg, and we can see a "blue screen"
    jmp     $   ; stop in here

load_msg:
    mov     bl, 54  ; each error msg has 54 bytes
    mul     bl  ; compute the right error msg offset
    add     eax, ErrorMsg - exception_report    ; eax point to the right error msg offset base on the seg
    mov     esi, eax    ; esi = eax
    mov     edi, 0;322  ; load the error msg print location to edi
    add     edi, CrashMsg - exception_report    ; edi point to the right error msg location offset base on seg
    mov     ecx, 54 ; the string has 54 bytes
    rep     movsb   ; copy the error msg to right location
    ret     ; function end

load_32reg:
    push    edi ; -. save the edi and ecx
    push    ecx ; -'
    mov     ecx, 0x8    ; 32reg has 8 bytes
    mov     edi, 0x8    ; the last char is offset 8
    jmp     load_reg_start  ; go to load the reg value
load_16reg:
    push    edi ; -. save the edi and ecx
    push    ecx ; -'
    mov     ecx, 0x4    ; 16reg has 4 bytes
    mov     edi, 0x4    ; the last char is offset 4
load_reg_start:
    add     edi, CrashMsg - exception_report    ; -. edi point to the right offset base on  this funtion
    add     edi, edx                            ; -'
    mov     edx, eax    ; copy al to dl
    and     dl, 00001111b   ; mask high 4-bit
load_reg_byte:
    add     byte [edi], dl  ; load one 4-bit value to right location
    cmp     dl, 0xa ; if the number is a, b, c, d, e, f
    jb      load_reg_num    ; no, it is only a number
    add     byte [edi], 0x27    ; yes, it isn't a number, and we need make is right
load_reg_num:
    dec     edi ; edi point to the front char
    shr     eax, 0x4    ; make higher 4-bit into al
    mov     dl, al  ; copy al to dl
    and     dl, 00001111b   ; mask high 4-bit
    loop    load_reg_byte   ; go on load the next 4-bit
    pop     ecx ; -. restore the edi and ecx
    pop     edi ; -'
    ret ; function end

load_reg_bit:
    mov     ecx, 0x20   ; eflags, cr0, cr2 and cr3 has 32bits
    mov     edi, 0x20                           ; -.
    add     edi, CrashMsg - exception_report    ;  | edi point to the location of the last bit
    add     edi, edx                            ; -'
    mov     edx, eax    ; copy the al to dl
    and     dl, 00000001b   ; mask the byte and only use last bit
load_reg_bit_byte:
    add     byte [edi], dl  ; load the bit to right location
    dec     edi ; edi point to the front bit
    shr     eax, 0x1    ; mov one bit to the last bit of al
    mov     dl, al  ; copy al to dl
    and     dl, 00000001b   ; mask the dl and only use last bit
    loop    load_reg_bit_byte   ; load the next bit
    ret ; function end

print_crash_msg:
    cld ; clear direction flag for next lobsb
    mov     ah, color_b_blue + color_white  ; set the back and front color
    xor     edi, edi    ; clear edi, let it point to the start of the video memory
print_crash_char:
    lodsb   ; load one char to al
    test    al, al  ; if the al is the end flag(0)
    jz      print_crash_end ; print end
    mov     [gs:edi], ax    ; write this char to video memory
    inc     edi ; point to the next char color value
    inc     edi ; point to the next char value
    jmp     print_crash_char    ; print the next char
print_crash_end:
    ret ; function end

DTR_struct:
DTR_limit dw 0x0
DTR_base  dd 0x0

VectorNumber db 0x0
ErrorCode    dd 0x0
SaveEIP      dd 0x0
SaveCS       dd 0x0
SaveEFLAGS   dd 0x0

ErrorMsg db \
"[#DE] Divide Error !!!                                ", \
"[#DB] Debug Error !!!                                 ", \
"[#NMI] Nonmaskable Interrupt !!!                      ", \
"[#INT3] Debug Interrupt !!!                           ", \
"[#OF] Overflow !!!                                    ", \
"[#BR] Exceed the Bound Range !!!                      ", \
"[#UD] Undefined Opcode !!!                            ", \
"[#NM] No Mathmetical Coprocessor !!!                  ", \
"[#DF] Double Fault !!!                                ", \
"[#CBR] Coprocessor Exceed the Bound Range !!!         ", \
"[#TS] TSS Error !!!                                   ", \
"[#NP] Segment Dose Not Present !!!                    ", \
"[#SS] Stack Segment Error !!!                         ", \
"[#GP] General protection Error !!!                    ", \
"[#PF] Page Fault !!!                                  ", \
"[#RS] Intel Reserved !!!                              ", \
"[#MF] 80x87 FPU Error !!!                             ", \
"[#AC] Aligment Check !!!                              ", \
"[#MC] Machine Check !!!                               ", \
"[#XF] SIMD Float Exception !!!                        "

CrashMsg db \
"|==============================================================================|", \
"|           ICT perfect 2.0 has crashed with a critical error !!!              |", \
"|==============================================================================|", \
"                                                                                ", \
"                                                        Error Code: 0x00000000  ", \
"                                                                                ", \
"  eax = 0x00000000    edx = 0x00000000    ecx = 0x00000000    ebx = 0x00000000  ", \
"  esp = 0x00000000    ebp = 0x00000000    esi = 0x00000000    edi = 0x00000000  ", \
"                                                                                ", \
"  es = 0x0000    ss = 0x0000    ds = 0x0000    fs = 0x0000    gs = 0x0000       ", \
"                                                                                ", \
"  eflags = 00000000000000000000000000000000                                     ", \
"                                                                                ", \
"  cr0 = 00000000000000000000000000000000                                        ", \
"  cr2 = 00000000000000000000000000000000                                        ", \
"  cr3 = 00000000000000000000000000000000                                        ", \
"                                                                                ", \
"                                                                                ", \
"  gdtr: base = 0x00000000, limit = 0x0000                                       ", \
"  idtr: base = 0x00000000, limit = 0x0000                                       ", \
"                                                                                ", \
"  cs = 0x0000    eip = 0x00000000                                               ", \
"                                                                                ", \
"                                                                                ", \
"                                                     Must restart computer !!!  ", 0x0

exception_report_length equ $ - exception_report

;==============================================================================;
; start page mechanism                                                         ;
;==============================================================================;
start_page:
    push    es  ; save es
    mov     ax, PG_selector ; ax = PG_selector
    mov     es, ax  ; load PG_selector to es
load_PDE:
    mov     eax, PG_LOCATION + 0x1000   ; eax point to the first PTE
    or      eax, PG_present | PG_rw | PG_spl    ; PDE attr
    mov     ecx, 0x400  ; have 1024 PDEs
    xor     edi, edi    ; edi = 0, point to the first PDE
    cld ; clear the direction flag
load_next_PDE:
    stosd   ; [es:edi] = eax
    add     eax, 0x1000 ; eax point to the next PDE
    loop    load_next_PDE   ; until load all 1024 PDEs
load_PTE:
    mov     eax, dword [MemorySize] ; load the MemorySize to eax, which got it in real mode
    xor     edx, edx    ; edx = 0, for next div
    mov     ebx, 0x1000 ; each PTE has 4096bytes
    div     ebx ; compute the sum of PTE we real have
    mov     edx, eax    ; save the sum of PTE to edx
    xor     eax, eax    ; eax = 0, we page the memory from 0x0 address
    or      eax, PG_present | PG_rw | PG_spl    ; PTE attr
    mov     ecx, 0x100000   ; we have 1024*1024 PTE, include virual memory
    mov     edi, 0x1000 ; the PTE start from 0x1000, for front 0x1000 space is PDEs 
load_next_present_PTE:
    stosd   ; [es:edi] = eax
    add     eax, 0x1000 ; point to the next 4kb memory page
    dec     edx ; edx--, we have loaded the a real memory PTE
    test    edx, edx    ; if we haven't real memory page, goto virtual page load subfunc
    jz      load_virtual_PTE    ; jump to the virtual page load subfunc
    loop    load_next_present_PTE   ; load the next real memory PTE
load_virtual_PTE:
    dec     ecx ; eax, for front load have not counted 
    test    ecx, ecx    ; if all pages is real memory page and have loaded, then load complete
    jz      open_page   ; open the page mechanism
    and     eax, 11111111111111111111000000000000b  ; mask the attr of real memory PTE
    or      eax, PG_rw | PG_spl ; set the attr of virtual memory PTE
load_next_virtual_PTE:
    stosd   ; [es:edi] = eax
    add     eax, 0x1000 ; eax point to the next 4kb virtual memory page
    loop    load_next_virtual_PTE   ; until load all 1024 * 1024 PTEs
open_page:
    mov     eax, PG_LOCATION    ; eax = PDE base address
    mov     cr3, eax    ; set it into PDBR in cr3
    mov     eax, cr0    ; eax = cr0
    or      eax, 10000000000000000000000000000000b  ; set the PG-bit to open page mechanism 
    mov     cr0, eax    ; reset the cr0 to let page mechanism works
    pop     es  ; restore es, and now the page mechanism is working
    ret ; function end, and cpu works with page mechanism

;==============================================================================;
; load and init kernel                                                         ;
;==============================================================================;
load_init_kernel:
    push    ds
    mov     ax, kernel_file_selector
    mov     ds, ax
    xor     ecx, ecx
    mov     cx, word [e_ehsize - EFLheader]
    xor     esi, esi
    mov     edi, EFLheader
    cld
    rep     movsb
    cmp     dword [es:e_ident], ELF_magic
    jne     load_init_kernel_notelf
    cmp     byte [es:e_type], 0x2
    jne     load_init_kernel_notexec
    cmp     byte [es:e_flags], 0x0
    jne     load_init_kernel_notia32
    mov     esi, dword [es:e_phoff]
load_one_segment:
    mov     edi, Programheader
    mov     cx, word [es:e_phentsize]
    rep     movsb
    push    esi
    push    es
    mov     eax, dword [es:p_type]
    test    eax, eax
    jz      load_next_segment
    mov     esi, dword [es:p_offset]
    mov     edi, dword [es:p_vaddr]
    mov     ecx, dword [es:p_filesz]
    mov     ax, memory_selector
    mov     es, ax
    rep     movsb
load_next_segment:
    pop     es
    pop     esi
    mov     ax, word [es:e_phnum]
    dec     ax
    test    ax, ax
    jz      load_segment_end
    mov     word [es:e_phnum], ax
    jmp     load_one_segment
load_segment_end:
    pop     ds
    ret
load_init_kernel_notelf:
    pop     ds
    call    print
    db  color_red, "The kernel.ict is not ELF file !!!", 0x0
    jmp     load_init_kernel_error
load_init_kernel_notexec:
    pop     ds
    call    print
    db  color_red, "The kernel.ict is not executable ELF file !!!", 0x0
    jmp     load_init_kernel_error
load_init_kernel_notia32:
    pop     ds
    call    print
    db  color_red, "The kernel.ict is not IA-32 ELF file !!!", 0x0
    jmp     load_init_kernel_error
load_init_kernel_error:
    jmp     $


%include "../kernel/ELFstruct.inc"
ELF_magic equ 0x464c457f

protected_sysldr_lenght equ $ - protected_sysldr
