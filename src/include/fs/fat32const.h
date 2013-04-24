/*================================================================================*/
/*                              ictOS FAT32 Constents                             */
/*                                                                        by: ict */
/*================================================================================*/

#define FAT_END 0x0fffffff
#define FAT_0   0xf8ffff0f
#define FAT_1   0xffffff0f

#define ROOT_FAT    0x2

#define ATTR_READONLY   0x01
#define ATTR_HIDE       0x02
#define ATTR_SYSTEM     0x04
#define ATTR_VOLUME     0x08
#define ATTR_DIR        0x10
#define ATTR_SAVE       0x20
#define ATTR_LNAME      0x0f

#define FLAG_CLEAN      0x00
#define FLAG_DEL        0xe5
#define FLAG_END        0x40

#define DIRENTRY_SIZE   0x20

#define SNAME_LEN       0xb
#define LNAME_LEN       0xff

#define LNAME_PER_ENTRY 0xd

#define DBR_OEM
#define DBR_SECTORS_PER_CLUSTER
#define DBR_RESERVED_SECTORS
#define DBR_FAT_SUM
#define DBR_SECTORS_SUM_SMALL
#define DBR_MEDIA
#define DBR_SECTORS_PER_TRACK
#define DBR_HEADS
#define DBR_OFFSET
#define DBR_SECTORS_SUM_BIG
#define DBR_FAT_SIZE
#define DBR_FLAG
#define DBR_VERSION
#define DBR_ROOT_CLUSTER
#define DBR_FSINFO
#define DBR_BACKUP_BOOT
#define DBR_DEVICE
#define DBR_EXTEND
#define DBR_VOLUME_NUMBER
#define DBR_VOLUME_TABLE
#define DBR_FS
