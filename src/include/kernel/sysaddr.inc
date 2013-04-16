;==============================================================================;
;                             ictOS System Address                             ;
;                                                                      by: ict ;
;==============================================================================;

GDT_LOCATION        equ     0x0
GDT_LIMIT           equ     0x1fff
MEMORY_LOCATION     equ     0x0
MEMORY_LIMIT        equ     0xfffff
STACK_LOCATION      equ     0x4000
STACK_LIMIT         equ     0x3bfff
VIDEO_LOCATION      equ     0xb8000
VIDEO_LIMIT         equ     0x7fff
IDT_LOCATION        equ     0x2000
IDT_LIMIT           equ     0x7ff
PG_LOCATION         equ     0x100000
PG_LIMIT            equ     0x400 + 0x1

KERNEL_SELECTOR     equ     0x8
