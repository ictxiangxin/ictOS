/*================================================================================*/
/*                       ICT Perfect 2.00 i8059a I/O Ports                        */
/*                                                                        by: ict */
/*================================================================================*/

#define MASTER_8259A_ICW1   0x20
#define SLAVE_8259A_ICW1    0xa0
#define MASTER_8259A        0x21
#define SLAVE_8259A         0xa1
#define MASTER_8259A_OCW1   0x21
#define SLAVE_8259A_OCW1    0xa1
#define MASTER_8259A_OCW2   0x20
#define SLAVE_8259A_OCW2    0xa0

#define IRQ_CLOCK           0xfe
#define IRQ_KEYBOARD        0xfd
#define IRQ_SLAVE8259A      0xfb
#define IRQ_COM1            0xf7
#define IRQ_COM2            0xef
#define IRQ_LPT2            0xdf
#define IRQ_FLOPPY          0xbf
#define IRQ_LPT1            0x7f
#define IRQ_NONE            0xff
#define IRQ_ALL             0x00

#define IRQ_ATA             0xbf
