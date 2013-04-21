/*================================================================================*/
/*                              ictOS FAT32 constents                             */
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
