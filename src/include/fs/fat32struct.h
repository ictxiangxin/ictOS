/*================================================================================*/
/*                              ictOS FAT32 Structure                             */
/*                                                                        by: ict */
/*================================================================================*/

#ifndef _FAT32STRUCT_H_
#define _FAT32STRUCT_H_

#include "type.h"

typedef struct dbr
{
    BYTE    jmp[0x3];
    BYTE    oem[0x8];
    WORD    bytes_per_sector;
    BYTE    sectors_per_cluster;
    WORD    reserved_sectors;
    BYTE    fat_sum;
    WORD    unused1;
    WORD    sectors_sum_small;
    BYTE    media;
    WORD    unused2;
    WORD    sectors_per_track;
    WORD    heads;
    DWORD   offset;
    DWORD   sectors_sum_big;
    DWORD   fat_size;
    WORD    flag;
    WORD    version;
    DWORD   root_cluster;
    WORD    fsinfo;
    WORD    backup_boot;
    BYTE    unused3[0xc];
    BYTE    device;
    BYTE    unused4;
    BYTE    extend;
    DWORD   volume_number;
    BYTE    volume_table[0xb];
    BYTE    fs[0x8];
} __attribute__((packed)) DBR;

typedef struct short_directory_entry
{
    BYTE    filename[0xb];
    BYTE    attr;
    BYTE    unused;
    BYTE    create_ots;
    WORD    create_time;
    WORD    create_date;
    WORD    access_date;
    WORD    cluster_high;
    WORD    change_time;
    WORD    change_date;
    WORD    cluster_low;
    DWORD   size;
} __attribute__((packed)) SDIRENTRY; 

typedef struct long_directory_entry
{
    BYTE    number;
    WORD    filename_1_5[0x5];
    BYTE    flag;
    BYTE    reserved1;
    BYTE    checksum;
    WORD    filename_6_11[0x6];
    WORD    reserved2;
    WORD    filename_12_13[0x2];
} __attribute__((packed)) LDIRENTRY;

#endif
