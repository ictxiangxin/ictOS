/*================================================================================*/
/*                       ICT Perfect 2.00 kernel file system                      */
/*                                                                        by: ict */
/*================================================================================*/

#include "type.h"
#include "constent.h"
#include "public.h"
#include "sig.h"
#include "servers.h"
#include "../fs/fat32struct.h"

PRIVATE FILEDESC* fdt; /* file description table */
PRIVATE FATBLOCK* fatbuff;
PRIVATE DBR*  fat32dbr;
PRIVATE BYTE* tmp_sector;

PRIVATE VOID init_kfs()
{
    fdt = ict_malloc(sizeof(FILEDESC) * FDT_SIZE);
    if(fdt == NULL)
        return; /* crash !!! */
    fatbuff = ict_malloc(sizeof(FATBLOCK) * FAT_BLOCK_SUM);
    if(fatbuff == NULL)
        return; /* crash !!!*/
    tmp_sector = ict_malloc(SECTOR_SIZE);
    if(tmp_sector == NULL)
        return; /* crash !!!*/
    fat32dbr = ict_malloc(sizeof(DBR));
    if(fat32dbr == NULL)
        return; /* crash !!!*/
}

PRIVATE VOID kfs_daemon()
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


