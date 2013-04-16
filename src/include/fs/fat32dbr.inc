;==============================================================================;
;                        ictOS 1.00 FAT32 DBR Structure                        ;
;                                                                      by: ict ;
;==============================================================================;

;\
| Data Label |___________| Type |_|   Data   |
dbr_oem_name                db      'XiangXin'
dbr_bytes_per_sector        dw      0x200
dbr_sectors_per_cluster     db      0x1
dbr_reserved_sectors        dw      0x20
dbr_fat_sum                 db      0x2
dbr_unused1                 dw      0x0
dbr_sectors_sum_small       dw      0x0
dbr_media                   db      0xf8
dbr_unused2                 dw      0x0
dbr_spt                     dw      0x3f
dbr_heads                   dw      0xff
dbr_offset                  dd      0x0
dbr_sectors_sum_big         dd      0x27de0
dbr_fat_size                dd      0x4e8
dbr_flag                    dw      0x0
dbr_version                 dw      0x0
dbr_root_cluster            dd      0x2
dbr_fsinfo                  dw      0x5
dbr_backup_boot             dw      0x6
dbr_unused3       times 0xc db      0x0
dbr_device                  db      0x80
dbr_unused4                 db      0x0
dbr_extend                  db      0x29
dbr_volume_number           dd      0x1715370A
dbr_volume_table            db      'ictOS 1.0.0'
dbr_fs                      db      'FAT32   '
