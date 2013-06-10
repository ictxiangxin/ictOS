;==============================================================================;
;                         ictOS 1.00 bootblock header                          ;
;                                                                      by: ict ;
;==============================================================================;

DBR_SECTORS_PER_CLUSTER equ     0xd
DBR_RESERVED_SECTORS    equ     0xe
DBR_FAT_SUM             equ     0x10
DBR_FAT_SIZE            equ     0x24
DBR_OFFSET              equ     0x1c
DBR_DEVICE              equ     0x40

DE_SIZE                 equ     0x20
DE_FILENAME             equ     0x0
DE_CLUSTER_LOW          equ     0x1a
DE_CLUSTER_HIGH         equ     0x14

ROOT_FAT_NUM            equ     0x2

FAT_BUFF_ADDR           equ     0x8000
FAT_BUFF_SIZE           equ     0x8
TMP_DATA_BASE           equ     0x1000

SYSLDR_BASE             equ     0x4000
SYSLDR_OFFSET           equ     0x0

KERNEL_BASE             equ     0x5000
KERNEL_OFFSET           equ     0x0
