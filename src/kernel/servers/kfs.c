/*================================================================================*/
/*                             ictOS kernel file system                           */
/*                                                                        by: ict */
/*================================================================================*/

#include "type.h"
#include "constent.h"
#include "public.h"
#include "sig.h"
#include "servers.h"
#include "kproc.h"
#include "../fs/fat32struct.h"
#include "../fs/fat32const.h"
#include "../io/ata.h"

PRIVATE VOID  _init_fa32_arg();
PRIVATE VOID  _init_fatcache();
PRIVATE VOID  _init_fdt();
PRIVATE DWORD _kfs_open_sname(DWORD kpid, BYTE* filepath, DWORD mode);
PRIVATE DWORD _kfs_open_lname(DWORD kpid, WORD* filepath, DWORD mode);
PRIVATE DWORD _find_file_sname(BYTE* filepath);
PRIVATE DWORD _find_file_lname(WORD* filepath);
PRIVATE BYTE* _next_filepath_sname(BYTE* filepath);
PRIVATE WORD* _next_filepath_lname(WORD* filepath);
PRIVATE DWORD _search_file_in_dir_sname(DWORD fat_num, BYTE* filename, SDIRENTRY* entrybuff);
PRIVATE DWORD _search_file_in_dir_lname(DWORD fat_num, WORD* filename, SDIRENTRY* entrybuff);
PRIVATE VOID  _setup_sname(BYTE* filename, BYTE* sname);
PRIVATE VOID  _build_lname(LDIRENTRY* entry, WORD* sname);
PRIVATE DWORD _read_cluster(DWORD cluster_num, BYTE* buff);
PRIVATE DWORD _next_fat(DWORD fat_num);
PRIVATE DWORD _alloc_fdt();
PRIVATE VOID  _free_fdt(DWORD fd_num);
PRIVATE DWORD _link_to_fdpblock(DWORD kpid, POINTER fdp);

PRIVATE FILEDESC* fdt; /* file description table */
PRIVATE FATBLOCK* fatcache;
PRIVATE DBR*      fat32dbr;

PRIVATE DWORD fat1offset;
PRIVATE DWORD fat2offset;
PRIVATE DWORD dataoffset;

PRIVATE BYTE* tmp_sector;
PRIVATE BYTE* tmp_cluster;

PRIVATE DWORD lock = FALSE;

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
}

PUBLIC VOID kfs_daemon()
{
    while(ict_lock(&lock))
        ict_done();
    _init_fa32_arg(); /* init fat32 arguments */
    ict_unlock(&lock);
    FCB* fcb;
    DWORD fp;
    MSG m;
    while(TRUE)
    {
        recv_msg(&m);
        while(ict_lock(&lock))
            ict_done();
        switch(m.sig)
        {
            case KFS_OPEN :
                fcb = (FCB*)m.data;
                if(fcb->namemode == SNAME_MODE)
                    fp = _kfs_open_sname(m.sproc_id, (BYTE*)fcb->filepath, fcb->openmode);
                else if(fcb->namemode == LNAME_MODE)
                    fp = _kfs_open_lname(m.sproc_id, (WORD*)fcb->filepath, fcb->openmode);
                else
                    fp = NULL;
                send_msg(m.sproc_id, KFS_OPENOVER, fp, NULL);
                break;
        }
        dest_msg ( &m );
        ict_unlock(&lock);
    }
}

PUBLIC VOID init_fdpblock(FDPBLOCK* fdpblock)
{
    DWORD i;
    for(i = 0; i < OPNEFILE_SUM; i++)
    {
        fdpblock[i].idle = TRUE;
        fdpblock[i].fdp = NULL;
    }
}

PUBLIC DWORD ict_open_sname(BYTE* filepath, DWORD mode)
{
    if(!ict_lock(&lock))
    {
        DWORD fp = _kfs_open_sname(ict_mypid(), filepath, mode);
        ict_unlock(&lock);
        return fp;
    }
    FCB fcb;
    fcb.namemode = SNAME_MODE;
    fcb.openmode = mode;
    fcb.filepath = filepath;
    MSG m;
    send_msg(PID_KFS, KFS_OPEN, (DWORD)&fcb, sizeof(FCB));
    return_msg(&m, PID_KFS, KFS_OPENOVER);
    return m.data;
}

PUBLIC DWORD ict_open_lname(WORD* filepath, DWORD mode)
{
    if(!ict_lock(&lock))
    {
        DWORD fp = _kfs_open_lname(ict_mypid(), filepath, mode);
        ict_unlock(&lock);
        return fp;
    }
    FCB fcb;
    fcb.namemode = LNAME_MODE;
    fcb.openmode = mode;
    fcb.filepath = filepath;
    MSG m;
    send_msg(PID_KFS, KFS_OPEN, (DWORD)&fcb, sizeof(FCB));
    return_msg(&m, PID_KFS, KFS_OPENOVER);
    return m.data;
}

PRIVATE VOID _init_fa32_arg()
{
    ict_hdread(0, 0x1, ATA_PRIMARY, tmp_sector);
    ict_memcpy(tmp_sector, fat32dbr, sizeof(DBR));
    tmp_cluster = (BYTE*)ict_malloc(fat32dbr->sectors_per_cluster * SECTOR_SIZE);
    if(tmp_cluster == NULL)
        return;
    fat1offset = fat32dbr->reserved_sectors + fat32dbr->offset;
    if(fat32dbr->fat_sum == 0x2)
    {
        fat2offset = fat1offset + fat32dbr->fat_size;
        dataoffset = fat2offset + fat32dbr->fat_size;
    }
    else
    {
        fat2offset = NULL;
        dataoffset = fat1offset + fat32dbr->fat_size;
    }
    _init_fatcache(); /* init fat cache */
    _init_fdt();
}

PRIVATE VOID _init_fatcache()
{
    DWORD i;
    for(i = 0; i < FAT_BLOCK_SUM; i++)
    {
        ict_hdread(fat1offset + i * (FAT_BLOCK_SIZE / SECTOR_SIZE), FAT_BLOCK_SIZE / SECTOR_SIZE, ATA_PRIMARY, &(fatcache[i].data));
        fatcache[i].id = i;
        fatcache[i].time = 0x1; /* min time is 1 */
    }
}

PRIVATE VOID _init_fdt()
{
    DWORD i;
    for(i = 0x0; i < FDT_SIZE; i++)
    {
        fdt[i].fat = NULL;
        fdt[i].mode = NULL;
        fdt[i].offset = NULL;
    }
}

PRIVATE DWORD _kfs_open_sname(DWORD kpid, BYTE* filepath, DWORD mode)
{
    DWORD fat_num;
    DWORD fd_num;
    DWORD fdp_num;
    if((fat_num = _find_file_sname(filepath)) == FALSE)
        return NULL; /* no such file */
    if((fd_num = _alloc_fdt()) == NONE)
        return NULL; /* fdt is full */
    fdt[fd_num].fat = fat_num;
    fdt[fd_num].mode = mode;
    fdt[fd_num].offset = 0x0;
    if((fdp_num =_link_to_fdpblock(kpid, &(fdt[fd_num]))) == NULL)
        return NULL; /* kproc can not open file */
    return fdp_num;
}

PRIVATE DWORD _kfs_open_lname(DWORD kpid, WORD* filepath, DWORD mode)
{
    DWORD fat_num;
    DWORD fd_num;
    DWORD fdp_num;
    if((fat_num = _find_file_lname(filepath)) == FALSE)
        return NULL; /* no such file */
    if((fd_num = _alloc_fdt()) == NONE)
        return NULL; /* fdt is full */
    fdt[fd_num].fat = fat_num;
    fdt[fd_num].mode = mode;
    fdt[fd_num].offset = 0x0;
    if((fdp_num =_link_to_fdpblock(kpid, &(fdt[fd_num]))) == NULL)
        return NULL; /* kproc can not open file */
    return fdp_num;
}

PRIVATE DWORD _find_file_sname(BYTE* filepath)
{
    SDIRENTRY tmp_entry;
    DWORD tmp_fat_num = ROOT_FAT;
    if(*filepath == '/')
        filepath++; /* clear '/' */
    BYTE* filename = filepath;
    BYTE* temp_filename = NULL;
    while(filename != NULL)
    {
        temp_filename = _next_filepath_sname(filename);
        if(_search_file_in_dir_sname(tmp_fat_num, filename, &tmp_entry) == FALSE)
            return FALSE; /* no such file */
        tmp_fat_num = 0x0;
        tmp_fat_num |= tmp_entry.cluster_high;
        tmp_fat_num <<= 0x10;
        tmp_fat_num |= tmp_entry.cluster_low;
        filename = temp_filename;
    }
    return tmp_fat_num;
}

PRIVATE DWORD _find_file_lname(WORD* filepath)
{
    SDIRENTRY tmp_entry;
    DWORD tmp_fat_num = ROOT_FAT;
    if(*filepath == '/')
        filepath++;
    WORD* filename = filepath;
    WORD* temp_filename = NULL;
    while(filename != NULL)
    {
        temp_filename = _next_filepath_lname(filename);
        if(_search_file_in_dir_lname(tmp_fat_num, filename, &tmp_entry) == FALSE)
            return FALSE; /* no such file */
        tmp_fat_num = 0x0;
        tmp_fat_num |= tmp_entry.cluster_high;
        tmp_fat_num <<= 0x10;
        tmp_fat_num |= tmp_entry.cluster_low;
        filename = temp_filename;
    }
    return tmp_fat_num;
}

PRIVATE BYTE* _next_filepath_sname(BYTE* filepath)
{
    while(*filepath != '/' && *filepath != '\0')
        filepath++;
    if(*filepath == '\0')
        return NULL; /* no next file */
    else
        *filepath = '\0';
    return ++filepath;
}

PRIVATE WORD* _next_filepath_lname(WORD* filepath)
{
    while(*filepath != '/' && *filepath != '\0')
        filepath++;
    if(*filepath == '\0')
        return NULL; /* no next file */
    else
        *filepath = '\0';
    return ++filepath;
}

PRIVATE DWORD _search_file_in_dir_sname(DWORD fat_num, BYTE* filename, SDIRENTRY* entrybuff)
{
    SDIRENTRY* item;
    BYTE tmp_sname[SNAME_LEN] = {0x0};
    while(fat_num != FAT_END)
    {
        _read_cluster(fat_num, tmp_cluster);
        item = (SDIRENTRY*)tmp_cluster;
        while((DWORD)item - (DWORD)tmp_cluster < fat32dbr->sectors_per_cluster * SECTOR_SIZE)
        {
            if(item->filename[0x0] == FLAG_CLEAN)
                return FALSE; /* no such file */
            if(item->filename[0x0] == FLAG_DEL)
                goto next_entry;
            if(item->attr == ATTR_LNAME)
                goto next_entry;
            _setup_sname(filename, tmp_sname);
            if(ict_strcmpl(tmp_sname, item->filename, SNAME_LEN) == TRUE)
            {
                ict_memcpy(item, entrybuff, sizeof(SDIRENTRY));
                return TRUE; /* have this file */
            }
        next_entry:
            item = (SDIRENTRY*)((DWORD)item + DIRENTRY_SIZE);
        }
        fat_num = _next_fat(fat_num);
    }
    return FALSE; /* no such file */
}

PRIVATE DWORD _search_file_in_dir_lname(DWORD fat_num, WORD* filename, SDIRENTRY* entrybuff)
{
    LDIRENTRY* item;
    DWORD find_one = FALSE;
    WORD tmp_lname[LNAME_LEN + 0x1] = {0x0};
    while(fat_num != FAT_END)
    {
        _read_cluster(fat_num, tmp_cluster);
        item = (LDIRENTRY*)tmp_cluster;
        while((DWORD)item - (DWORD)tmp_cluster < fat32dbr->sectors_per_cluster * SECTOR_SIZE)
        {
            if(item->number == FLAG_CLEAN)
                return FALSE; /* no such file */
            if(item->number == FLAG_DEL)
                goto next_entry;
            if(item->flag != ATTR_LNAME)
                if(find_one == FALSE)
                    goto next_entry;
                else
                {
                    if(ict_ustrcmp(filename, tmp_lname) == TRUE)
                    {
                        ict_memcpy(item, entrybuff, sizeof(SDIRENTRY));
                        return TRUE; /* have this file */
                    }
                    find_one = FALSE;
                    goto next_entry;
                }
            find_one = TRUE;
            _build_lname(item, tmp_lname);
        next_entry:
            item = (LDIRENTRY*)((DWORD)item + DIRENTRY_SIZE);
        }
        fat_num = _next_fat(fat_num);
    }
    return FALSE; /* no such file */
}

PRIVATE VOID _setup_sname(BYTE* filename, BYTE* sname)
{
    DWORD len = ict_strlen(filename);
    DWORD i;
    for(i = 0x0; i < SNAME_LEN; i++)
    {
        if(*filename == '.')
        {
            for(; i < SNAME_LEN - 0x3; i++)
                sname[i] = ' ';
            filename++;
        }
        sname[i] = *filename;
        if(*filename == '\0')
            for(; i < SNAME_LEN; i++)
                sname[i] = ' ';
        filename++;
    }
}

PRIVATE VOID _build_lname(LDIRENTRY* entry, WORD* lname)
{
    DWORD num = entry->number & 0x1f;
    num--;
    num *= LNAME_PER_ENTRY;
    DWORD i;
    for(i = 0x0; i < 0x5; i++)
        if(entry->filename_1_5[i] != '\0')
            lname[num + i] = entry->filename_1_5[i];
    for(i = 0x0; i < 0x6; i++)
        if(entry->filename_6_11[i] != '\0')
            lname[num + i + 0x5] = entry->filename_6_11[i];
    for(i = 0x0; i < 0x2; i++)
        if(entry->filename_12_13[i] != '\0')
            lname[num + i + 0xb] = entry->filename_12_13[i];
}

PRIVATE DWORD _read_cluster(DWORD cluster_num, BYTE* buff)
{
    DWORD sector_num;
    DWORD err;
    cluster_num -= 0x2; /* data area start with cluster 2 */
    sector_num = cluster_num * fat32dbr->sectors_per_cluster;
    sector_num += dataoffset;
    err = ict_hdread(sector_num, fat32dbr->sectors_per_cluster, ATA_PRIMARY, buff);
    return err;
}

PRIVATE DWORD _next_fat(DWORD fat_num)
{
    if(fat_num == FAT_END)
        return fat_num;
    DWORD fat_id =   fat_num & 0xffffff80;
    DWORD fat_addr = fat_num & 0x0000007f;
    DWORD cache_num = NONE;
    DWORD i;
    for(i = 0x0; i < FAT_BLOCK_SUM; i++)
        if(fatcache[i].id == fat_id)
        {
            cache_num = i;
            break;
        }
    if(cache_num == NONE) /* this FAT item not in FAT cache */
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
    return fatcache[i].data[fat_addr];
}

PRIVATE DWORD _alloc_fdt()
{
    DWORD i;
    for(i = 0x0; i < FDT_SIZE; i++)
        if(fdt[i].fat == NULL)
            return i;
    return NONE;
}

PRIVATE VOID _free_fdt(DWORD fd_num)
{
    fdt[fd_num].fat = NULL;
    fdt[fd_num].mode = NULL;
    fdt[fd_num].offset = NULL;
}

PRIVATE DWORD _link_to_fdpblock(DWORD kpid, POINTER fdp)
{
    DWORD i;
    for(i = 0x1; i < OPNEFILE_SUM; i++)
        if(ict_pcb(kpid)->fdpblock[i].idle == TRUE)
        {
            ict_pcb(kpid)->fdpblock[i].idle = FALSE;
            ict_pcb(kpid)->fdpblock[i].fdp = fdp;
            return i;
        }
    return NULL; /* fdp block full */
}
