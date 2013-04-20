/*================================================================================*/
/*                             ictOS kernel file system                           */
/*                                                                        by: ict */
/*================================================================================*/

#include "type.h"
#include "constent.h"
#include "public.h"
#include "sig.h"
#include "servers.h"
#include "../fs/fat32struct.h"
#include "../fs/fat32const.h"

PRIVATE FILEDESC* fdt; /* file description table */
PRIVATE FATBLOCK* fatcache;
PRIVATE DBR*      fat32dbr;
PRIVATE DWORD     fat1offset;
PRIVATE DWORD     fat2offset;
PRIVATE DWORD     dataoffset;
PRIVATE DWORD     rootfat;

PRIVATE BYTE* tmp_sector;
PRIVATE BYTE* tmp_cluster;

PUBLIC VOID init_kfs()
{
    fdt = (FILEDESC*)ict_malloc(sizeof(FILEDESC) * FDT_SIZE);
    if(fdt == NULL)
        return; /* crash !!! */
    fatcache = (FATBLOCK*)ict_malloc(sizeof(FATBLOCK) * FAT_BLOCK_SUM);
    if(fatcache == NULL)
        return; /* crash !!!*/
    tmp_sector = (BYTE*)ict_malloc(SECTOR_SIZE);
    if(tmp_sector == NULL)
        return; /* crash !!!*/
    fat32dbr = (DBR*)ict_malloc(sizeof(DBR));
    if(fat32dbr == NULL)
        return; /* crash !!!*/
    ict_hdread(0, 1, 0, tmp_sector);
    ict_memcpy(tmp_sector, fat32dbr, sizeof(DBR));
    tmp_cluster = (BYTE*)ict_malloc(fat32dbr.sectors_per_cluster * SECTOR_SIZE);
    if(tmp_cluster == NULL)
        return;
    fat1offset = fat32dbr.reserved_sectors + fat32dbr.offset;
    if(fat32dbr.fat_sum == 0x2)
    {
        fat2offset = fat1offset + fat32dbr.fat_size;
        dataoffset = fat2offset + fat32dbr.fat_size;
    }
    else
    {
        fat2offset = NULL;
        dataoffset = fat1offset + fat32dbr.fat_size;
    }
    init_fatcache(); /* init fat cache */
}

PUBLIC VOID kfs_daemon()
{
	MSG m;
	while(TRUE)
	{
		recv_msg(&m);
		switch(m.sig)
		{
		}
        dest_msg ( &m );
	}
}

PRIVATE VOID init_fatcache()
{
    DWORD i;
    for(i = 0; i < FAT_BLOCK_SUM; i++)
    {
        ict_hdread(fat2offset + i, &(fatcache[i].data), 0x1);
        fatcache[i].id = i;
        fatcache[i].time = 0x1; /* min time is 1 */
    }
}

PRIVATE DWORD _kfs_open(BYTE* filepath)
{
}

PRIVATE DWORD _search_file_in_dir(DWORD fat_num, BYTE* filename, DWORD filetype)
{
    _read_cluster(fat_num, tmp_cluster);
    /* the next is to compare file name */
}

PRIVATE DWORD _read_cluster(DWORD cluster_num, BYTE* buff)
{
    DWORD sector_num;
    DWORD err;
    cluster_num -= 0x2; /* data area start with cluster 2 */
    sector_num = cluster_num * fat32dbr.sectors_per_cluster;
    sector_num += dataoffset;
    err = ict_hdread(sector_num, fat32dbr.sectors_per_cluster, ATA_PRIMARY, buff);
    return err;
}

PRIVATE DWORD _next_fat(DWORD fat_num)
{
    if(fat_num == FAT_END)
        return fat_num;
    DWORD fat_id =   fat_num & 0xffffff80;
    DWORD fat_addr = fat_num & 0x0000007f;
    DWORD cache_num = 0xffffffff;
    DWORD i;
    for(i = 0x0; i < FAT_BLOCK_SUM; i++)
        if(fatcache[i].id == fat_id)
        {
            cache_num = i;
            break;
        }
    if(cache_num == 0xffffffff) /* this FAT item not in FAT cache */
    {
        DWORD max_time_id = 0x0;
        for(i = 0x0; i < FAT_BLOCK_SUM; i++)
            if(fatcache[max_time_id].time < fatcache[i].time)
                max_time_id = i;
        fatcache[max_time_id].id = fat_id; /* set it to this fat id */
        fatcache[max_time_id].time = 0x0; /* reset the no access time */
        DWORD sector_num = fat_num / FAT_BLOCK_SIZE * 0x4 + fat1offset;
        if(ict_hdread(sector_num, FAT_BLOCK_SIZE / SECTOR_SIZE, ATA_PRIMARY, fatcache[max_time_id].data) != NULL)
            return NULL; /* crash */
        cache_num = max_time_id;
    }
    /* increase all time */
    for(i = 0x0; i < FAT_BLOCK_SUM; i++)
        fatcache[i].time++;
    return facache[i].data[fat_addr];
}

