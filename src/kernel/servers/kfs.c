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

PRIVATE VOID  _init_fat32_arg();
PRIVATE VOID  _init_fatcache();
PRIVATE VOID  _init_fdt();
PRIVATE VOID  _init_febt();
PRIVATE DWORD _ict_open(POINTER filepath, DWORD mode, DWORD namemode);
PRIVATE DWORD _kfs_open(DWORD kpid, POINTER filepath, DWORD mode, DWORD namemode);
PRIVATE VOID  _kfs_close(DWORD kpid, DWORD fp);
PRIVATE DWORD _kfs_read(DWORD kpid, DWORD fp, DWORD size, POINTER buff);
PRIVATE DWORD _kfs_seek(DWORD kpid, DWORD fp, LINT offset, DWORD origin);
PRIVATE DWORD _find_file_sname(BYTE* filepath, SDIRENTRY* entry);
PRIVATE DWORD _find_file_lname(WORD* filepath, SDIRENTRY* entry);
PRIVATE BYTE* _next_filepath_sname(BYTE* filepath);
PRIVATE WORD* _next_filepath_lname(WORD* filepath);
PRIVATE DWORD _search_file_in_dir_sname(DWORD fat_num, BYTE* filename, SDIRENTRY* entrybuff);
PRIVATE DWORD _search_file_in_dir_lname(DWORD fat_num, WORD* filename, SDIRENTRY* entrybuff);
PRIVATE VOID  _setup_sname(BYTE* filename, BYTE* sname);
PRIVATE VOID  _build_lname(LDIRENTRY* entry, WORD* sname);
PRIVATE DWORD _read_cluster(DWORD cluster_num, BYTE* buff, DWORD sum);
PRIVATE DWORD _cluster_string(DWORD *fat_num, DWORD maxlen);
PRIVATE DWORD _next_fat(DWORD fat_num);
PRIVATE DWORD _alloc_fdt();
PRIVATE VOID  _free_fdt(DWORD fd_num);
PRIVATE DWORD _alloc_febt(DWORD cluster, DWORD num);
PRIVATE VOID  _free_febt(DWORD feb_num);
PRIVATE DWORD _link_to_fdpblock(DWORD kpid,DWORD fdnum);

PRIVATE FDESC*    fdt; /* file description table */
PRIVATE FEB*      febt; /* FEB table */
PRIVATE FATBLOCK* fatcache;
PRIVATE DBR*      fat32dbr;
PRIVATE DWORD     clustersize;

PRIVATE DWORD fat1offset;
PRIVATE DWORD fat2offset;
PRIVATE DWORD dataoffset;

PRIVATE BYTE* tmp_sector;
PRIVATE BYTE* tmp_cluster;
PRIVATE DWORD tmp_fatnum; /* used by febt */
PRIVATE DWORD tmp_entrynum; /* used by febt */

PRIVATE DWORD lock = FALSE;

PUBLIC VOID init_kfs()
{
    if((fdt = (FDESC*)ict_malloc(sizeof(FDESC) * FDT_SIZE)) == NULL)
        return; /* crash !!! */
    if((fatcache = (FATBLOCK*)ict_malloc(sizeof(FATBLOCK) * FAT_BLOCK_SUM)) == NULL)
        return; /* crash !!!*/
    if((tmp_sector = (BYTE*)ict_malloc(SECTOR_SIZE)) == NULL)
        return; /* crash !!!*/
    if((fat32dbr = (DBR*)ict_malloc(sizeof(DBR))) == NULL)
        return; /* crash !!!*/
    if((febt = (FEB*)ict_malloc(sizeof(FEB) * FEBT_SIZE)) == NULL)
        return; /* crash !!!*/
}

PUBLIC VOID kfs_daemon()
{
    while(ict_lock(&lock))
        ict_done();
    _init_fat32_arg(); /* init fat32 arguments */
    ict_unlock(&lock);
    FCB* fcb;
    RWCB* rwcb;
    SCB* scb;
    DWORD fp;
    DWORD rwsize;
    DWORD loc;
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
                fp = _kfs_open(m.sproc_id, (BYTE*)fcb->filepath, fcb->openmode, fcb->namemode);
                send_msg(m.sproc_id, KFS_OPENOVER, fp, NULL);
                break;
            case KFS_CLOSE :
                _kfs_close(m.sproc_id, m.data);
                break;
            case KFS_READ :
                rwcb = (RWCB*)m.data;
                rwsize = _kfs_read(m.sproc_id, rwcb->fp, rwcb->size, rwcb->buff);
                send_msg(m.sproc_id, KFS_READOVER, rwsize, NULL);
                break;
            case KFS_SEEK :
                scb = (SCB*)m.data;
                loc = _kfs_seek(m.sproc_id, scb->fp, scb->offset, scb->origin);
                send_msg(m.sproc_id, KFS_SEEKOVER, loc, NULL);
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
        fdpblock[i].idle  = TRUE;
        fdpblock[i].fdnum = 0x0;
    }
}

PUBLIC FDESC* ict_fd(DWORD fdnum)
{
    return &(fdt[fdnum]);
}

PUBLIC DWORD ict_open_sname(BYTE* filepath, DWORD mode)
{
    return _ict_open(filepath, mode, SNAME_MODE);
}

PUBLIC DWORD ict_open_lname(WORD* filepath, DWORD mode)
{
    return _ict_open(filepath, mode, LNAME_MODE);
}

PUBLIC DWORD ict_read(DWORD fp, DWORD size, POINTER buff)
{
    if(!ict_lock(&lock))
    {
        DWORD readsize = _kfs_read(ict_mypid(), fp, size, buff);
        ict_unlock(&lock);
        return readsize;
    }
    RWCB rwcb;
    rwcb.fp = fp;
    rwcb.size = size;
    rwcb.buff = buff;
    MSG m;
    send_msg(PID_KFS, KFS_READ, (DWORD)&rwcb, sizeof(RWCB));
    return_msg(&m, PID_KFS, KFS_READOVER);
    return m.data;
}

PUBLIC DWORD ict_seek(DWORD fp, DWORD offset, DWORD origin)
{
    if(!ict_lock(&lock))
    {
        DWORD loc = _kfs_seek(ict_mypid(), fp, offset, origin);
        ict_unlock(&lock);
        return loc;
    }
    SCB scb;
    scb.fp =fp;
    scb.offset = offset;
    scb.origin = origin;
    MSG m;
    send_msg(PID_KFS, KFS_SEEK, (DWORD)&scb, sizeof(SCB));
    return_msg(&m, PID_KFS, KFS_SEEKOVER);
    return m.data;
}

PUBLIC VOID ict_close(DWORD fp)
{
    if(!ict_lock(&lock))
    {
        _kfs_close(ict_mypid(), fp);
        ict_unlock(&lock);
    }
    send_msg(PID_KFS, KFS_CLOSE, fp, NULL);
}

PRIVATE VOID _init_fat32_arg()
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
    clustersize = fat32dbr->sectors_per_cluster * SECTOR_SIZE;
    _init_fatcache(); /* init fat cache */
    _init_fdt();
    _init_febt();
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

PRIVATE VOID _init_febt()
{
    DWORD i;
    for(i = 0x0; i < FEBT_SIZE; i++)
    {
        febt[i].idle = TRUE;
        febt[i].fatnum = NONE;
        febt[i].entrynum = NONE;
        febt[i].access = 0x0;
    }
}

PRIVATE DWORD _ict_open(POINTER filepath, DWORD mode, DWORD namemode)
{
    if(!ict_lock(&lock))
    {
        DWORD fp = _kfs_open(ict_mypid(), filepath, mode, namemode);
        ict_unlock(&lock);
        return fp;
    }
    FCB fcb;
    fcb.namemode = namemode;
    fcb.openmode = mode;
    fcb.filepath = filepath;
    MSG m;
    send_msg(PID_KFS, KFS_OPEN, (DWORD)&fcb, sizeof(FCB));
    return_msg(&m, PID_KFS, KFS_OPENOVER);
    return m.data;
}

PRIVATE DWORD _kfs_open(DWORD kpid, POINTER filepath, DWORD mode, DWORD namemode)
{
    DWORD fat_num;
    DWORD fd_num;
    DWORD feb_num;
    DWORD fdp_num;
    SDIRENTRY tmp_entry;
    if(
       (fat_num = 
        namemode == SNAME_MODE ? 
        _find_file_sname((BYTE*)filepath, &tmp_entry) : _find_file_lname((WORD*)filepath, &tmp_entry)
       )
       == FALSE
      )
        return NULL; /* no such file */
    if((fd_num = _alloc_fdt()) == NONE)
        return NULL; /* fdt is full */
    if((fdp_num =_link_to_fdpblock(kpid, fd_num)) == NULL)
    {
        _free_fdt(fd_num);
        return NULL; /* kproc can not open file */
    }
    if((feb_num = _alloc_febt(tmp_fatnum, tmp_entrynum)) == NONE)
    {
        _free_fdt(fd_num);
        _free_febt(feb_num);
        return NULL; /* febt is full */
    }
    ict_memcpy(&tmp_entry, &(febt[feb_num].entry), DIRENTRY_SIZE);
    fdt[fd_num].fat = fat_num;
    fdt[fd_num].febnum = feb_num;
    fdt[fd_num].mode = mode;
    fdt[fd_num].offset = 0x0;
    return fdp_num;
}

PRIVATE VOID _kfs_close(DWORD kpid, DWORD fp)
{
    _free_febt(ict_fd(ict_pcb(kpid)->fdpblock[fp].fdnum)->febnum);
    _free_fdt(ict_pcb(kpid)->fdpblock[fp].fdnum);
    ict_pcb(kpid)->fdpblock[fp].idle = TRUE;
    ict_pcb(kpid)->fdpblock[fp].fdnum = 0x0;
}

PRIVATE DWORD _kfs_read(DWORD kpid, DWORD fp, DWORD size, POINTER buff)
{
    if(ict_pcb(kpid)->fdpblock[fp].idle == TRUE)
        return NONE;
    FDESC* fd = ict_fd(ict_pcb(kpid)->fdpblock[fp].fdnum);
    FEB* feb = &(febt[fd->febnum]);
    DWORD fat_num = fd->fat;
    DWORD tmp_fat = fat_num;
    BYTE* _buff = buff;
    DWORD cstrlen; /* cluster string length */
    if(size > feb->entry.size - fd->offset)
        size = feb->entry.size - fd->offset;
    if(size == 0x0)
        return size;
    DWORD skipcluster = fd->offset / clustersize;
    DWORD skipsize = fd->offset % clustersize;
    DWORD readcluster = (size + skipsize - clustersize) / clustersize;
    DWORD lastsize = size % clustersize;
    DWORD skipcount = 0x0;
    DWORD readcount = 0x0;
    if(size + skipsize < clustersize) /* if less than 1 cluster */
        readcluster = 0x0;
    fd->offset += size;
    while(fat_num != FAT_END && skipcluster != skipcount)
    {
        cstrlen = _cluster_string(&fat_num, skipcluster - skipcount);
        skipcount += cstrlen;
    }
    if(fat_num != FAT_END)
    {
        _read_cluster(fat_num, tmp_sector, 0x1);
        ict_memcpy(tmp_sector + skipsize, _buff, size < clustersize ? size : (clustersize - skipsize));
        _buff += clustersize - skipsize;
    }
    while(fat_num != FAT_END && readcluster != readcount)
    {
        cstrlen = _cluster_string(&fat_num, readcluster - readcount);
        readcount += cstrlen;
        _read_cluster(tmp_fat, _buff, cstrlen);
        _buff += cstrlen * clustersize;
        tmp_fat = fat_num;
    }
    if(fat_num != FAT_END && lastsize != 0x0)
    {
        _read_cluster(fat_num, tmp_sector, 0x1);
        ict_memcpy(tmp_sector, _buff, lastsize);
    }
    return size;
}

PRIVATE DWORD _kfs_seek(DWORD kpid, DWORD fp, LINT offset, DWORD origin)
{
    if(ict_pcb(kpid)->fdpblock[fp].idle == TRUE)
        return NONE;
    FDESC* fd = ict_fd(ict_pcb(kpid)->fdpblock[fp].fdnum);
    FEB* feb = &(febt[fd->febnum]);
    switch(origin)
    {
        case SEEK_START :
            if(offset < 0x0)
                fd->offset = 0x0;
            else
                fd->offset = offset;
            break;
        case SEEK_CURRENT :
            if(offset < 0x0)
                if(-offset > fd->offset)
                    fd->offset = 0x0;
                else
                    fd->offset += offset;
            else
                if(offset + fd->offset > feb->entry.size)
                    fd->offset = feb->entry.size;
                else
                    fd->offset += offset;
            break;
        case SEEK_END :
            if(offset > 0x0)
                fd->offset = feb->entry.size;
            else if(-offset > fd->offset)
                fd->offset = 0x0;
            else
                fd->offset = feb->entry.size + offset;
            break;
    }
    return fd->offset;
}

PRIVATE DWORD _find_file_sname(BYTE* filepath, SDIRENTRY* entry)
{
    DWORD tmp_fat_num = ROOT_FAT;
    if(*filepath == '/')
        filepath++; /* clear '/' */
    BYTE* filename = filepath;
    BYTE* temp_filename = NULL;
    while(filename != NULL)
    {
        temp_filename = _next_filepath_sname(filename);
        if(_search_file_in_dir_sname(tmp_fat_num, filename, entry) == FALSE)
            return FALSE; /* no such file */
        tmp_fat_num = 0x0;
        tmp_fat_num |= entry->cluster_high;
        tmp_fat_num <<= 0x10;
        tmp_fat_num |= entry->cluster_low;
        filename = temp_filename;
    }
    return tmp_fat_num;
}

PRIVATE DWORD _find_file_lname(WORD* filepath, SDIRENTRY* entry)
{
    DWORD tmp_fat_num = ROOT_FAT;
    if(*filepath == '/')
        filepath++;
    WORD* filename = filepath;
    WORD* temp_filename = NULL;
    while(filename != NULL)
    {
        temp_filename = _next_filepath_lname(filename);
        if(_search_file_in_dir_lname(tmp_fat_num, filename, entry) == FALSE)
            return FALSE; /* no such file */
        tmp_fat_num = 0x0;
        tmp_fat_num |= entry->cluster_high;
        tmp_fat_num <<= 0x10;
        tmp_fat_num |= entry->cluster_low;
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
        _read_cluster(fat_num, tmp_cluster, 0x1);
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
                tmp_fatnum = fat_num;
                tmp_entrynum = ((DWORD)item - (DWORD)tmp_cluster) / DIRENTRY_SIZE;
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
        _read_cluster(fat_num, tmp_cluster, 0x1);
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
                        tmp_fatnum = fat_num;
                        tmp_entrynum = ((DWORD)item - (DWORD)tmp_cluster) / DIRENTRY_SIZE;
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

PRIVATE DWORD _read_cluster(DWORD cluster_num, BYTE* buff, DWORD sum)
{
    DWORD sector_num;
    DWORD err;
    cluster_num -= 0x2; /* data area start with cluster 2 */
    sector_num = cluster_num * fat32dbr->sectors_per_cluster;
    sector_num += dataoffset;
    err = ict_hdread(sector_num, fat32dbr->sectors_per_cluster * sum, ATA_PRIMARY, buff);
    return err;
}

PRIVATE DWORD _cluster_string(DWORD *fat_num, DWORD maxlen)
{
    DWORD sum = 0x0;
    DWORD tmp_fat = *fat_num;
    DWORD current_fat;
    do
    {
        sum++;
        current_fat = tmp_fat;
        tmp_fat = _next_fat(current_fat);
    } while(tmp_fat - current_fat == 0x1 && sum <= maxlen);
    *fat_num = tmp_fat;
    return sum;
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
    fdt[fd_num].febnum = NULL;
    fdt[fd_num].mode = NULL;
    fdt[fd_num].offset = NULL;
}

PRIVATE DWORD _alloc_febt(DWORD fatnum, DWORD entrynum)
{
    DWORD i;
    for(i = 0x0; i < FEBT_SIZE; i++)
        if(febt[i].idle == FALSE)
            if(febt[i].fatnum == fatnum && febt[i].entrynum == entrynum)
            {
                febt[i].access++;
                return i;
            }
    for(i = 0x0; i < FEBT_SIZE; i++)
        if(febt[i].idle == TRUE)
        {
            febt[i].idle = FALSE;
            febt[i].fatnum = fatnum;
            febt[i].entrynum = entrynum;
            febt[i].access = 0x1;
            return i;
        }
    return NONE;
}

PRIVATE VOID _free_febt(DWORD feb_num)
{
    febt[feb_num].access--;
    if(febt[feb_num].access == 0x0)
    {
        febt[feb_num].idle = TRUE;
        febt[feb_num].fatnum = NONE;
        febt[feb_num].entrynum = NONE;
    }
}

PRIVATE DWORD _link_to_fdpblock(DWORD kpid,DWORD fdnum)
{
    DWORD i;
    for(i = 0x1; i < OPNEFILE_SUM; i++)
        if(ict_pcb(kpid)->fdpblock[i].idle == TRUE)
        {
            ict_pcb(kpid)->fdpblock[i].idle = FALSE;
            ict_pcb(kpid)->fdpblock[i].fdnum = fdnum;
            return i;
        }
    return NULL; /* fdp block full */
}
