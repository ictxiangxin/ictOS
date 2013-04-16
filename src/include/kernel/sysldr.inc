;==============================================================================;
;                          ictOS Contents of sysldr.s                          ;
;                                                                      by: ict ;
;==============================================================================;

KERNEL_BASE             equ     0x5000
KERNEL_OFFSET           equ     0x0
KERNEL_LIMIT            equ     0xffff
KERNEL_LOCATION         equ     0x501000

DATA_LIMIT              equ     0xfffff

GDTR_length             equ     0x6

EXCEPT_LOCATION         equ     0x2800
EXCEPT_LIMIT            equ     0x17ff

DAT_KERNEL              equ     DAT_x       + DAT_present + DAT_datacode + DAT_32a32i + DAT_g_4kb
DAT_MEMORY              equ     DAT_rw      + DAT_present + DAT_datacode + DAT_g_4kb + DAT_32a32i
DAT_STACK_SEG           equ     DAT_rw      + DAT_present + DAT_datacode
DAT_VIDEO_SEG           equ     DAT_rw      + DAT_present + DAT_datacode
DAT_EXCEPT              equ     DAT_rw      + DAT_present + DAT_datacode
DAT_EXCEPT_EXEC         equ     DAT_x       + DAT_present + DAT_datacode + DAT_32a32i
DAT_TSS                 equ     DAT_e386tss + DAT_present

DAT_PROTECTED_SYSLDR    equ     DAT_x       + DAT_present + DAT_datacode + DAT_32a32i
DAT_DATA_SEG            equ     DAT_rw      + DAT_present + DAT_datacode + DAT_g_4kb
DAT_TEMP_GDT            equ     DAT_rw      + DAT_present + DAT_datacode 
DAT_REAL_GDT            equ     DAT_rw      + DAT_present + DAT_datacode
DAT_IDT                 equ     DAT_rw      + DAT_present + DAT_datacode
DAT_PG                  equ     DAT_rw      + DAT_present + DAT_datacode + DAT_g_4kb
DAT_KERNEL_FILE         equ     DAT_rw      + DAT_present + DAT_datacode + DAT_g_4kb
DAT_INTERRUPT           equ     DAT_80386igate
