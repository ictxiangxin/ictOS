/*================================================================================*/
/*                         ictOS Kernel Memory Management                         */
/*                                                                        by: ict */
/*================================================================================*/

#include "constent.h"
#include "type.h"
#include "sig.h"
#include "public.h"
#include "kproc.h"
#include "../io/BIOScolor.h"

PRIVATE MEMBLOCK*  usedlist; /* point to used list */
PRIVATE MEMBLOCK*  idlelist; /* point to idle list */
PRIVATE MEMRECORD* recordlist; /* point to first record list */
PRIVATE MEMRECORD* nowrecord; /* current record pointer */

PRIVATE DWORD lock = FALSE; /* memory lock */

PRIVATE VOID    mem_init();
PRIVATE VOID    record_add(MEMBLOCK* list, POINTER addr, DWORD size);
PRIVATE VOID    record_remove(MEMBLOCK* node);
PRIVATE POINTER _ict_malloc(DWORD size);
PRIVATE DWORD   _ict_free(POINTER addr);
PRIVATE DWORD   _ict_idlesize();

/******************************************************************/
/* init the memory management service, and create two init nodes  */
/******************************************************************/
PUBLIC VOID init_mem()
{
    recordlist = (MEMRECORD*)MEM_FREEMEM_ADDR;
    recordlist->count = 0x2;
    recordlist->statmap = 0x3;
    recordlist->back = recordlist;
    recordlist->next = recordlist;
    recordlist->spacelist[0].addr = (POINTER)MEM_FREEMEM_ADDR;
    recordlist->spacelist[0].size = sizeof(MEMRECORD);
    recordlist->spacelist[0].back = &(recordlist->spacelist[0]);
    recordlist->spacelist[0].next = &(recordlist->spacelist[0]);
    recordlist->spacelist[1].addr = (POINTER)(MEM_FREEMEM_ADDR + sizeof(MEMRECORD));
    recordlist->spacelist[1].size = MEM_SIZE - MEM_FREEMEM_ADDR - sizeof(MEMRECORD);
    recordlist->spacelist[1].back = &(recordlist->spacelist[1]);
    recordlist->spacelist[1].next = &(recordlist->spacelist[1]);
    usedlist = &(recordlist->spacelist[0]);
    idlelist = &(recordlist->spacelist[1]);
    nowrecord = recordlist;
} 

/******************************************************************/
/* malloc for kproc                                               */
/******************************************************************/
PUBLIC POINTER ict_malloc(DWORD size)
{
    if(!ict_lock(&lock)) /* test and lock */
    {
        POINTER rst = _ict_malloc(size);
        ict_unlock(&lock);
        return rst;
    }
    MSG m;
    send_msg(PID_MEM, MEM_MALLOC, (DWORD)size, NULL);
    return_msg(&m, PID_MEM, MEM_ANS);
    return (POINTER)m.data;
}

/******************************************************************/
/* free for kproc                                                 */
/******************************************************************/
PUBLIC DWORD ict_free(POINTER addr)
{
    if(!ict_lock(&lock)) /* test and lock */
    {
        DWORD rst = _ict_free(addr);
        ict_unlock(&lock);
        return rst;
    }
    MSG m;
    send_msg(PID_MEM, MEM_FREE, (DWORD)addr, NULL);
    return_msg(&m, PID_MEM, MEM_ANS);
    return m.data;
}

/******************************************************************/
/* special malloc only used by msg service                        */
/******************************************************************/
PUBLIC POINTER msg_malloc(DWORD size)
{
    while(ict_lock(&lock))
        ict_done(); /* until lock successfully */
    POINTER rst = _ict_malloc(size);
    ict_unlock(&lock);
    return rst;
}

/******************************************************************/
/* special free only used by msg service                          */
/******************************************************************/
PUBLIC DWORD msg_free(POINTER addr)
{
    while(ict_lock(&lock))
        ict_done(); /* until lock successfully */
    DWORD rst = _ict_free(addr);
    ict_unlock(&lock);
    return rst;
}

/******************************************************************/
/* get the idle space of memory                                   */
/******************************************************************/
PUBLIC DWORD ict_idlesize()
{
    if(!ict_lock(&lock)) /* test and lock */
    {
        DWORD rst = _ict_idlesize();
        ict_unlock(&lock);
        return rst;
    }
    MSG m;
    send_msg(PID_MEM, MEM_IDLE, NULL, NULL);
    return_msg(&m, PID_MEM, MEM_ANS);
    return m.data;
}

/******************************************************************/
/* memory management daemon                                       */
/******************************************************************/
PUBLIC VOID mem_daemon()
{
    MSG m;
    DWORD tmp;
    while(TRUE)
    {
        recv_msg(&m);
        while(!ict_lock(&lock))
            ict_done(); /* until lock successfully */
        switch(m.sig)
        {
        case MEM_MALLOC :
            tmp = (DWORD)_ict_malloc(m.data);
            send_msg(m.sproc_id, MEM_ANS, tmp, NULL);
            break;
        case MEM_FREE :
            tmp = _ict_free((POINTER)m.data);
            send_msg(m.sproc_id, MEM_ANS, tmp, NULL);
            break;
        case MEM_IDLE :
            tmp = _ict_idlesize();
            send_msg(m.sproc_id, MEM_ANS, tmp, NULL);
            break;
        }
        dest_msg ( &m );
        ict_unlock(&lock);
    }
}

/******************************************************************/
/* add a linklist node to current record                          */
/******************************************************************/
PRIVATE VOID record_add(MEMBLOCK* list, POINTER addr, DWORD size)
{
    MEMBLOCK* tmp_scan = list;
    MEMBLOCK* dest_node;
    DWORD test_map = 0x1;
    DWORD test_num = 0;
    for(test_num = 0; nowrecord->statmap & test_map; test_num++)
        test_map <<= 1;
    dest_node = &(nowrecord->spacelist[test_num]);
    dest_node->addr = addr;
    dest_node->size = size;
    while(!(tmp_scan->addr < addr && (tmp_scan->next->addr > addr || tmp_scan->next == list)))
        if(tmp_scan->next == list)
        {
            dest_node->next = list;
            dest_node->back = list->back;
            list->back->next = dest_node;
            list->back = dest_node;
            if(list == idlelist)
                idlelist = dest_node;
            else
                usedlist = dest_node;
            nowrecord->statmap |= test_map;
            nowrecord->count++;
            return;
        }
        else
            tmp_scan = tmp_scan->next;
    dest_node->next = tmp_scan->next;
    dest_node->back = tmp_scan;
    tmp_scan->next->back = dest_node;
    tmp_scan->next = dest_node;
    nowrecord->statmap |= test_map;
    nowrecord->count++;
    return;
}

/******************************************************************/
/* remove a linklist node from a record                           */
/******************************************************************/
PRIVATE VOID record_remove(MEMBLOCK* node)
{
    MEMRECORD* scan = recordlist;
    DWORD test_map = 0x1;
    DWORD test_num = 0;
    while(!((DWORD)scan < (DWORD)node && (DWORD)scan + sizeof(MEMRECORD) > (DWORD)node))
        if(scan->next == recordlist)
            return; /* never go to here */
        else
            scan = scan->next;
    for(test_num = 0; &(scan->spacelist[test_num]) != node; test_num++)
        test_map <<= 1;
    scan->statmap ^= test_map;
    scan->count--;
    if(scan->count == 0x0)
    {
        scan->back->next = scan->next;
        scan->next->back = scan->back;
        _ict_free((POINTER)node);
    }
}

/******************************************************************/
/* main malloc function                                           */
/******************************************************************/
PRIVATE POINTER _ict_malloc(DWORD size)
{
    MEMBLOCK* idle_scan = idlelist;
    while(idle_scan->size < size) /* find a memory block, which size >= required size */
        if(idle_scan->next == idlelist) /* not enough memory */
            return NULL;
        else
            idle_scan = idle_scan->next;
    /* find it */
    if(idle_scan->size == size)
    {
        MEMBLOCK* used_scan = usedlist;
        while(!(used_scan->addr < idle_scan->addr && (used_scan->next->addr > idle_scan->addr || used_scan->next == usedlist)))
            if(used_scan->next == usedlist)
            {
                if(idle_scan == idlelist)
                    idlelist = idle_scan->next;
                idle_scan->back->next = idle_scan->next;
                idle_scan->next->back = idle_scan->back;
                idle_scan->back = usedlist->back;
                idle_scan->next = usedlist;
                usedlist->back->next = idle_scan;
                usedlist->back = idle_scan;
                usedlist = idle_scan;
                return idle_scan->addr;
            }
            else
                used_scan = used_scan->next;
        if(idle_scan == idlelist)
            idlelist = idle_scan->next;
        idle_scan->back->next = idle_scan->next;
        idle_scan->next->back = idle_scan->back;
        idle_scan->back = used_scan;
        idle_scan->next = used_scan->next;
        used_scan->next->back = idle_scan;
        used_scan->next = idle_scan;
        return idle_scan->addr;
    }
    POINTER tmp_addr = idle_scan->addr;
    record_add(usedlist, tmp_addr, size);
    idle_scan->addr += size;
    idle_scan->size -= size;
    if(nowrecord->count == MEMRECORD_SPACESUM - 1)
    {
       MEMRECORD* tmp_record = (MEMRECORD*)_ict_malloc(sizeof(MEMRECORD));
       tmp_record->count = 0x0;
       tmp_record->statmap = 0x0;
       tmp_record->next = recordlist;
       tmp_record->back = recordlist->back;
       recordlist->back->next = tmp_record;
       recordlist->back = tmp_record;
       recordlist = tmp_record;
       nowrecord = tmp_record;
    }
    return tmp_addr;
}

/******************************************************************/
/* main free function                                             */
/******************************************************************/
PRIVATE DWORD _ict_free(POINTER addr)
{
    MEMBLOCK* used_scan = usedlist;
    MEMBLOCK* idle_scan = idlelist->back;
    while(used_scan->addr != addr)
        if(used_scan->next == usedlist)
            return FALSE;
        else
            used_scan = used_scan->next;
    while(idle_scan->addr > addr)
        if(idle_scan == idlelist)
            break;
        else
            idle_scan = idle_scan->back;
    if(idle_scan->addr > addr)
        if(used_scan->next->addr == addr + used_scan->size)
        {
            used_scan->back->next = used_scan->next;
            used_scan->next->back = used_scan->back;
            used_scan->back = idlelist->back;
            used_scan->next = idlelist;
            idlelist->back->next = used_scan;
            idlelist->back = used_scan;
            idlelist = used_scan;
            return TRUE;
        }
        else if(idle_scan->addr == addr + used_scan->size)
        {
            idlelist->addr -= used_scan->size;
            idlelist->size += used_scan->size;
            used_scan->back->next = used_scan->next;
            used_scan->next->back = used_scan->back;
            record_remove(used_scan);
            return TRUE;
        }
        else
            return FALSE; /* never go to here */
    if(idle_scan->addr + idle_scan->size == addr)
        if(used_scan->next->addr == addr + used_scan->size || addr + used_scan->size == (POINTER)MEM_SIZE)
        {
            idle_scan->size += used_scan->size;
            used_scan->back->next = used_scan->next;
            used_scan->next->back = used_scan->back;
            record_remove(used_scan);
            return TRUE;
        }
        else if (idle_scan->next->addr == addr + used_scan->size)
        {
            idle_scan->size += used_scan->size + idle_scan->next->size;
            used_scan->back->next = used_scan->next;
            used_scan->next->back = used_scan->back;
            record_remove(used_scan);
            MEMBLOCK* tmp_idle = idle_scan->next;
            tmp_idle->back->next = tmp_idle->next;
            tmp_idle->next->back = tmp_idle->back;
            record_remove(tmp_idle);
            return TRUE;
        }
        else
            return FALSE; /* never go to here */
    if(idle_scan->addr + idle_scan->size < addr)
        if(used_scan->next->addr == addr + used_scan->size || addr + used_scan->size == (POINTER)MEM_SIZE)
        {
            used_scan->back->next = used_scan->next;
            used_scan->next->back = used_scan->back;
            used_scan->next = idle_scan->next;
            used_scan->back = idle_scan;
            idle_scan->next->back = used_scan;
            idle_scan->next = used_scan;
            return TRUE;
        }
        else if(idle_scan->next->addr == addr + used_scan->size)
        {
            idle_scan->next->addr -= used_scan->size;
            idle_scan->next->size += used_scan->size;
            used_scan->back->next = used_scan->next;
            used_scan->next->back = used_scan->back;
            record_remove(used_scan);
            return TRUE;
        }
        else
            return FALSE; /* never go to here */
}

/******************************************************************/
/* main idle size function                                        */
/******************************************************************/
PRIVATE DWORD _ict_idlesize()
{
    MEMBLOCK* idle_scan = idlelist->next;
    DWORD size = idlelist->size;
    while(idle_scan != idlelist)
    {
        size += idle_scan->size;
        idle_scan = idle_scan->next;
    }
    return size;
}
