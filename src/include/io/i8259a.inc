;==============================================================================;
;                           ictOS i8059a I/O Ports                             ;
;                                                                      by: ict ;
;==============================================================================;

MASTER_8259A_ICW1   equ     0x20
SLAVE_8259A_ICW1    equ     0xa0
MASTER_8259A        equ     0x21
SLAVE_8259A         equ     0xa1
MASTER_8259A_OCW1   equ     0x21
SLAVE_8259A_OCW1    equ     0xa1

IRQ_CLOCK           equ     11111110b
IRQ_KEYBOARD        equ     11111101b
IRQ_SLAVE8259A      equ     11111011b
IRQ_COM1            equ     11110111b
IRQ_COM2            equ     11101111b
IRQ_LPT2            equ     11011111b
IRQ_FLOPPY          equ     10111111b
IRQ_LPT1            equ     01111111b
IRQ_NONE            equ     11111111b
IRQ_ALL             equ     00000000b
