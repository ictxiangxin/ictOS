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

PRIVATE FILEDESC* fdt; /* file description table */
PRIVATE FATBLOCK* fatcache;
PRIVATE DBR*      fat32dbr;
PRIVATE DWORD     fat1offset;
PRIVATE DWORD     fat2offset;
PRIVATE DWORD     dataoffset;

PRIVATE BYTE* tmp_sector;

PUBLIC VOID init_kfs()
{
    fdt = ict_malloc(sizeof(FILEDESC) * FDT_SIZE);
    if(fdt == NULL)
        return; /* crash !!! */
    fatcache = ict_malloc(sizeof(FATBLOCK) * FAT_BLOCK_SUM);
    if(fatcache == NULL)
        return; /* crash !!!*/
    tmp_sector = ict_malloc(SECTOR_SIZE);
    if(tmp_sector == NULL)
        return; /* crash !!!*/
    fat32dbr = ict_malloc(sizeof(DBR));
    if(fat32dbr == NULL)
        return; /* crash !!!*/
    ict_hdread(0, 1, 0, tmp_sector);
    ict_memcpy(tmp_sector, fat32dbr, sizeof(DBR));
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
        fatcache[i].time = 0x0;
    }
}


