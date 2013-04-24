/*================================================================================*/
/*                               ictOS Global Constent                            */
/*                                                                        by: ict */
/*================================================================================*/

#ifndef _CONSTENT_H_
#define _CONSTENT_H_


#define TRUE                    0x1
#define FALSE                   0x0
#define NULL                    0x0
#define NONE                    0xffffffff

#define LDT_ELEMSUM             0x4
#define MSGBUF_SIZE             0x10
#define MSGBUF_SUM              0x10
#define KPROC_SUM               0x10

#define KEYBOARD_BUFFER_SIZE    0x20

#define DEFAULT_COLOR           COLOR_WHITE
#define GRAPHIC_BUFF_SIZE       0x2000
#define PRINT_BUFF              0x100

#define OPNEFILE_SUM            0x20
#define FDT_SIZE                0x1000
#define FEBT_SIZE               FDT_SIZE
#define FAT_BLOCK_SIZE          0x200
#define FAT_BLOCK_SUM           0x10
#define SECTOR_SIZE             0x200

#define FMODE_R                 0x1
#define FMODE_W                 0x2
#define FMODE_RW                0x3

#define LNAME_MODE              0x1
#define SNAME_MODE              0x2

#define SEEK_START              0x0
#define SEEK_CURRENT            0x1
#define SEEK_END                0x2

#define DESC_SIZE               0x8

#define RESERVED_DESC_BY_KERNEL 0x8

#define KPROC_STACK_BASE        0x600000
#define KPROC_STACK_SIZE        0x1000

#define MEM_FREEMEM_ADDR        0x700000
#define MEMRECORD_SPACESUM      0x20
#define MEM_SIZE                0x40000000

#endif
