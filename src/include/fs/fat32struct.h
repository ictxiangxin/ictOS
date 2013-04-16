/*================================================================================*/
/*                              ictOS FAT32 structure                             */
/*                                                                        by: ict */
/*================================================================================*/

#ifndef _FAT32STRUCT_H_
#define _FAT32STRUCT_H_

#include "type.h"

typedef struct dbr
{
    BYTE[0x3]   jmp;
    BYTE[0x8]   oem;
    WORD        bytes_per_sector;
    BYTE        sectors_per_cluster;
    WORD        reserved_sectors;
    BYTE        fat_sum;
    WORD        unused1;
    WORD        sectors_sum_small;
    BYTE        media;
    WORD        unused2;
    WORD        sectors_per_track;
    WORD        heads;
    DWORD       offset;
    DWORD       sectors_sum_big;
    DWORD       fat_size;
    WORD        flag;
    WORD        version;
    DWORD       root_cluster;
    WORD        fsinfo;
    WORD        backup_boot;
    BYTE[0xc]   unused3;
    BYTE        device;
    BYTE        unused4;
    BYTE        extend;
    DWORD       volume_number;
    BYTE[0xb]   volume_table;
    BYTE[0x8]   fs;
} DBR;

typedef struct short_directory_entry
{
    BYTE[0xb]   filename;
    BYTE        attr;
    BYTE        unused;
    BYTE        create_ots;
    WORD        create_time;
    WORD        create_date;
    WORD        access_date;
    WORD        cluster_high;
    WORD        change_time;
    WORD        change_date;
    WORD        cluster_low;
    DWORD       size;
} SDIRENTRY; 

typedef struct long_directory_entry
{
    BYTE        number;
    WORD[0x5]   filename_1_5;
    BYTE        flag;
    BYTE        reserved1;
    BYTE        checksum;
    WORD[0x6]   filename_6_11;
    WORD        reserved2;
    WORD[0x2]   filename_12_13;
} LDIRENTRY;

#endif
