/*================================================================================*/
/*                           ictOS IPC message service                            */
/*                                                                        by: ict */
/*================================================================================*/

#include "kasm.h"
#include "public.h"
#include "constent.h"
#include "type.h"
#include "sig.h"
#include "kproc.h"
#include "servers.h"
#include "../io/BIOScolor.h"

#define INIT_IDLE_BUF 0
#define HOOK_IDLE     -1
#define EMPTY_READ_P  -1
#define EMPTY_WRITE_P 0
#define IDLE_USEUP    0

PUBLIC KPROCLIST kernelproclist; /* the kernel proc list */
PUBLIC KPROC*    current_proc; /* save the addr of the kproc of the current proc */

PRIVATE MSGPOOL* p_kernelmsgpool; /*pointer of kernel msg pool */

PRIVATE DWORD lock = FALSE; /* lock of msg pool */

PRIVATE VOID init_msgpool ( MSGPOOL* msgpool );
PRIVATE VOID msgbuf_drop ( DWORD proc_id );
PRIVATE VOID null_msg(MSG* msg);

/******************************************************************/
/* init the msg pool, all buf will set to unused and packaged     */
/******************************************************************/
PUBLIC VOID init_msg()
{
    //ict_cprintf ( COLOR_YELLOW, "Setup message service...        " );
    p_kernelmsgpool = ( MSGPOOL* ) msg_malloc ( sizeof ( MSGPOOL ) ); /* get new space of msg pool */
    init_msgpool ( p_kernelmsgpool ); /* init the msg pool */
    /* build msgpool linklist */
    p_kernelmsgpool->back = p_kernelmsgpool;
    p_kernelmsgpool->next = p_kernelmsgpool;
    //ict_cprintf ( COLOR_YELLOW, "[ OK ]\n" );
}

/******************************************************************/
/* destory one msg if it used extra memory                        */
/******************************************************************/
PUBLIC VOID dest_msg ( MSG* msg )
{
    if ( msg->datasize == NULL )
        return;
    msg_free ( (POINTER)msg->data );
    msg->data = NULL;
    msg->datasize = NULL;
}

/******************************************************************/
/* receive the msg, if there are no msg, then wait                */
/******************************************************************/
PUBLIC VOID recv_msg ( MSG* msg )
{
    if ( have_msg() == FALSE ) /* if there are no msg */
    {
        send_msg ( PID_KPM, KPM_WAITMSG, NULL, NULL ); /* tell the kpm that I will wait for msg */
        ict_done(); /* nothing to do */
    }
    read_msg ( msg ); /* when I awake, it means have msg to me */
}

/******************************************************************/
/* receive the msg from a special proc                            */
/******************************************************************/
PUBLIC VOID return_msg(MSG* msg, DWORD src_proc_id, DWORD sig)
{
    if ( have_msg() == FALSE ) /* if there are no msg */
    {
        send_msg ( PID_KPM, KPM_WAITMSG, NULL, NULL ); /* tell the kpm that I will wait for msg */
        ict_done(); /* nothing to do */
    }
    while(!search_msg(msg, src_proc_id, sig))
        ict_done();
}

/******************************************************************/
/* if the proc have msg in its msg buf hooklist                   */
/******************************************************************/
PUBLIC BYTE have_msg()
{
    return current_proc->havemsg;   /* just return the flag of "have msg" */
}

/******************************************************************/
/* if the proc have int for it before                             */
/******************************************************************/
PUBLIC BYTE have_int()
{
    return current_proc->haveint; /* just return the flag of "have msg" */
}

/******************************************************************/
/* hook a new buf to the hooklist of proc                         */
/******************************************************************/
PUBLIC VOID msgbuf_hook ( DWORD proc_id )
{
    while(ict_lock(&lock))
        ict_done(); /* until lock */
    if ( p_kernelmsgpool->idle == IDLE_USEUP ) /* if the msg pool was use up */
    {
        MSGPOOL* newmsgpool = ( MSGPOOL* ) msg_malloc ( sizeof ( MSGPOOL ) );
        if ( newmsgpool == NULL )
        {
            ict_cprintf ( COLOR_RED, "Critical Error: out of memory!!!\n" );
            goto msgbuf_hook_end;
        }
        init_msgpool ( newmsgpool );
        newmsgpool->next = p_kernelmsgpool->next;
        newmsgpool->back = p_kernelmsgpool;
        p_kernelmsgpool->next->back = newmsgpool;
        p_kernelmsgpool->next = newmsgpool;
        p_kernelmsgpool = newmsgpool;
    }
    if ( kernelproclist.procs[proc_id].msgentry == 0 ) /* if the proc was not init its msg hooklist */
    {
        /* init the hooklist of this proc */
        kernelproclist.procs[proc_id].msgentry = p_kernelmsgpool->idle; /* give it a idle buf */
        p_kernelmsgpool->idle->hook = proc_id; /* mark the buf was hooked on this proc */
        kernelproclist.procs[proc_id].havemsg = FALSE;
    }
    else /* if this task is a normal hook buf */
    {
        MSGBUF* entry = kernelproclist.procs[proc_id].msgentry; /* get the hook entry of the proc */
        MSGBUF* last = entry; /* temp pointer for next loop */
        while ( last->next != entry ) /* find the end buf of this hook, which its next buf is entry of hooklist */
            last = last->next; /* test the next buf */
        last->next = p_kernelmsgpool->idle; /* hook a idle buf to the end of hooklist */
        last->next->next = entry; /* make the new buf link to the start */
        p_kernelmsgpool->idle->hook = proc_id; /* mark the buf was hooked on this proc */
    }
    DWORD i; /* for next loop */
    for ( i = 0; i < MSGBUF_SUM; i++ ) /* scan all the msg pool */
        if ( p_kernelmsgpool->msgbuflist[i].hook == HOOK_IDLE ) /* if find a unhook buf */
        {
            p_kernelmsgpool->idle = & ( p_kernelmsgpool->msgbuflist[i] ); /* make it become a idle buf */
            goto msgbuf_hook_end; /* function end */
        }
    /* when execute here, it means all buf were hooked on some procs */
    p_kernelmsgpool->idle = IDLE_USEUP; /* set the idle = IDLE_USEUP means no idle buf in msg pool */
msgbuf_hook_end:
    ict_unlock(&lock);
}

/******************************************************************/
/* init the msg pool, all buf will set to unused and packaged     */
/******************************************************************/
PRIVATE VOID init_msgpool ( MSGPOOL* msgpool )
{
    DWORD i = 0; /* for next loop */
    for ( i = 0; i < MSGBUF_SUM; i++ ) /* init all bufs of the msg pool */
    {
        msgpool->msgbuflist[i].hook = HOOK_IDLE; /* mark the buf to unhooked */
        msgpool->msgbuflist[i].next = &msgpool->msgbuflist[i]; /* package this buf */
        msgpool->msgbuflist[i].read_p = EMPTY_READ_P; /* it means the buf is empty */
        msgpool->msgbuflist[i].write_p = EMPTY_WRITE_P; /* set the write pointer point to the first */
    }
    msgpool->idle = & ( msgpool->msgbuflist[INIT_IDLE_BUF] ); /* make the first buf be the idle buf */
}

/******************************************************************/
/* drop a buf from the hooklist of the proc                       */
/******************************************************************/
PRIVATE VOID msgbuf_drop ( DWORD proc_id )
{
    while(ict_lock(&lock))
        ict_done(); /* until lock */
    /* entry point to the proc msg hook start */
    MSGBUF* entry = kernelproclist.procs[proc_id].msgentry;
    MSGBUF* temp = entry; /* temp pointer for next loop */
    MSGBUF* last = entry; /* restore the back pointer of the temp pointer */
    while ( last->next != entry ) /* find the last buf of this hooklist */
        last = last->next; /* test the next buf */
    if ( entry->read_p == EMPTY_READ_P ) /* if the buf is empty */
    {
        if ( entry->next != entry ) /* test if the buf is the only buf of this hook list */
            /* reset the msg entry of the proc */
            kernelproclist.procs[proc_id].msgentry = entry->next;
        else
            goto msgbuf_drop_end; /* the buf is the only buf, so do nothing */
        last->next = kernelproclist.procs[proc_id].msgentry; /* remove the buf from hooklist */
        entry->hook = HOOK_IDLE; /* reset the buf to idle */
        entry->next = temp; /* package itself */
        entry->write_p = EMPTY_WRITE_P; /* reset the write pointer point to the start */
        MSGPOOL* dropmsgpool = p_kernelmsgpool;
        while ( dropmsgpool->next != p_kernelmsgpool )
            if ( ( DWORD ) entry >= ( DWORD ) dropmsgpool && ( DWORD ) entry <= ( DWORD ) ( dropmsgpool + sizeof ( MSGPOOL ) ) )
                break;
            else
                dropmsgpool = dropmsgpool->next;
        DWORD i = 0;
        for ( i = 0; i < MSGBUF_SUM; i++ ) /* scan all the msg pool */
            if ( dropmsgpool->msgbuflist[i].hook != HOOK_IDLE ) /* if find a unhook buf */
                goto msgbuf_drop_end;
        if ( dropmsgpool->back == dropmsgpool && dropmsgpool->next == dropmsgpool ) /* only msg pool */
            goto msgbuf_drop_end; /* never go to here */
        MSGPOOL* tmp = dropmsgpool->next;
        dropmsgpool->next->back = dropmsgpool->back;
        dropmsgpool->back->next = dropmsgpool->next;
        msg_free ( dropmsgpool );
        if ( dropmsgpool == p_kernelmsgpool )
            p_kernelmsgpool = tmp;
    }
msgbuf_drop_end:
    ict_unlock(&lock);
}

/******************************************************************/
/* send a msg from this proc to other proc                        */
/******************************************************************/
PUBLIC DWORD send_msg ( DWORD dest_proc_id, DWORD sig, DWORD data, DWORD datasize )
{
    if ( kernelproclist.procs[dest_proc_id].status == KPS_SLEEP )    /* if the proc is sleep */
    {
        ict_sti();  /* all works done, open interrupt */
        return FALSE;   /* can not send msg to it */
    }
    while(ict_lock(&(kernelproclist.procs[dest_proc_id].msglock)))
        ict_done();
    MSGBUF* entry = kernelproclist.procs[dest_proc_id].msgentry;
    MSGBUF* temp_ent = entry; /* temp pointer for next loop */
    MSG temp_msg; /* create a temp msg for some init */
    temp_msg.sproc_id = current_proc->id; /* set the source proc to this proc */
    temp_msg.dproc_id = dest_proc_id; /* set the destination proc */
    temp_msg.sig = sig; /* set the sig of this msg */
    temp_msg.datasize = datasize;
    temp_msg.data = data; /* set the sig of this msg */
    while ( temp_ent->next != entry ) /* find the last buf */
        temp_ent = temp_ent->next; /* test next */
    if ( temp_ent->read_p == temp_ent->write_p ) /* if the last buf is full */
    {
        msgbuf_hook ( dest_proc_id ); /* hook a new buf to the proc from msg pool */
        temp_ent = temp_ent->next; /* point to the new buf */
    }
    if ( datasize != NULL )
    {
        POINTER tmpdata = ( POINTER ) msg_malloc ( datasize );
        ict_memcpy ( ( POINTER ) data, tmpdata, datasize );
        temp_msg.data = ( DWORD ) tmpdata;
    }
    /* if execute here, it means it find a buf that has space to store msg */
    /* copy the msg to destination proc msg buf first */
    ict_memcpy ( ( POINTER ) &temp_msg, ( POINTER ) & ( temp_ent->msglist[temp_ent->write_p] ), sizeof ( MSG ) );
    if ( temp_ent->read_p == EMPTY_READ_P ) /* if the buf is empty, all empty buf's read_p is -1 */
        temp_ent->read_p = temp_ent->write_p; /* make the read pointer usable */
    temp_ent->write_p++; /* move the write pointer to front */
    temp_ent->write_p %= MSGBUF_SIZE; /* rollback */
    kernelproclist.procs[dest_proc_id].havemsg = TRUE; /* set the flag of "have msg" */
    if ( kernelproclist.procs[dest_proc_id].status == KPS_WAITMSG )
        kernelproclist.procs[dest_proc_id].status = KPS_OK;
    ict_unlock(&(kernelproclist.procs[dest_proc_id].msglock));
    return TRUE; /* send msg successful */
}

/******************************************************************/
/* read a msg from it buf hooklist                                */
/******************************************************************/
PUBLIC DWORD read_msg ( MSG* msg )
{
    while(ict_lock(&(current_proc->msglock)))
        ict_done();
    /* entry point to the this proc msg hook start */
    MSGBUF* entry = current_proc->msgentry;
    MSGBUF* temp_ent = entry; /* temp pointer for next loop */
    if ( temp_ent->read_p == EMPTY_READ_P ) /* if the buf is empty */
    {
        null_msg(msg);
        ict_unlock(&(current_proc->msglock));
        return FALSE; /* read fail, and return false */
    }
    /* if execute here, it means it find a buf that has msg */
    /* copy the msg to receive space first */
    ict_memcpy ( ( POINTER ) & ( temp_ent->msglist[temp_ent->read_p] ), ( POINTER ) msg, sizeof ( MSG ) );
    /* move the read pointer to front first,
       and test if the buf is empty */
    temp_ent->read_p++; /* move the read pointer to front */
    temp_ent->read_p %= MSGBUF_SIZE; /* rollback */
    if ( temp_ent->read_p == temp_ent->write_p ) /* if the buf is empty */
    {
        temp_ent->read_p = EMPTY_READ_P; /* the buf is empty now, and set the read_p to -1 */
        msgbuf_drop ( current_proc->id ); /* drop the empty buf to msg pool */
    }
    if ( current_proc->msgentry->read_p == EMPTY_READ_P ) /* if the all msgs have been read */
        current_proc->havemsg = FALSE; /* clear the flag of "have msg" */
    ict_unlock(&(current_proc->msglock));
    if(msg->sproc_id == NULL && msg->dproc_id == NULL && msg->sig == NULL) /* this is a useless msg */
        read_msg(msg);  /* read next msg */
    return TRUE; /* it means this function execute perfect */
}

/******************************************************************/
/* search a special msg and read it                               */
/******************************************************************/
PUBLIC DWORD search_msg( MSG* msg, DWORD src_proc_id, DWORD sig )
{
    while(ict_lock(&(current_proc->msglock)))
        ict_done();
    /* entry point to the this proc msg hook start */
    MSGBUF* entry = current_proc->msgentry;
    MSGBUF* temp_ent = entry; /* temp pointer for next loop */
    DWORD   temp_read_p = temp_ent->read_p;
    if(temp_read_p == EMPTY_READ_P)
    {
        null_msg(msg);
        ict_unlock(&(current_proc->msglock));
        return FALSE; /* search fail, and return false */
    }
    while(temp_ent->msglist[temp_read_p].sproc_id != src_proc_id || temp_ent->msglist[temp_read_p].sig != sig)
    {
        temp_read_p++; /* move the read pointer to front */
        temp_read_p %= MSGBUF_SIZE; /* rollback */
        if(temp_read_p == temp_ent->write_p)
        {
            if(temp_ent->next == entry)
            {
                null_msg(msg);
                ict_unlock(&(current_proc->msglock));
                return FALSE; /* search fail, and return false */
            }
            temp_ent = temp_ent->next;
            temp_read_p = temp_ent->read_p;
        }
    }
    /* find the special msg */
    ict_memcpy ( ( POINTER ) & ( temp_ent->msglist[temp_read_p] ), ( POINTER ) msg, sizeof ( MSG ) );
    /* set this msg useless */
    temp_ent->msglist[temp_read_p].sproc_id = NULL;
    temp_ent->msglist[temp_read_p].dproc_id = NULL;
    temp_ent->msglist[temp_read_p].sig = NULL;
    ict_unlock(&(current_proc->msglock));
    return TRUE;
}

/******************************************************************/
/* clear the msg, drop all buf to msg pool, only reserve one      */
/******************************************************************/
PUBLIC VOID clear_msg()
{
    while(ict_lock(&(current_proc->msglock)))
        ict_done();
    /* entry point to the this proc msg hook start */
    MSGBUF* entry = current_proc->msgentry;
    MSGBUF* temp_ent = entry; /* temp pointer for next loop */
    MSGBUF* next_ent = entry->next; /* next pointer for next loop */
    while ( next_ent != entry ) /* if exist another buf in this hook list */
    {
        temp_ent = next_ent; /* point to the buf */
        next_ent = temp_ent->next; /* record its next buf */
        temp_ent->hook = HOOK_IDLE; /* mark it as a idle buf, put back to msg pool */
        temp_ent->next = temp_ent; /* pakage it */
        temp_ent->read_p = EMPTY_READ_P; /* reset the read pointer */
        temp_ent->write_p = EMPTY_WRITE_P; /* reset the write pointer */
    }
    entry->next = entry; /* only reserve one buf */
    entry->read_p = EMPTY_READ_P; /* reset the read pointer */
    entry->write_p = EMPTY_WRITE_P; /* reset the write pointer */
    ict_unlock(&(current_proc->msglock));
}

/******************************************************************/
/* load a null msg                                                */
/******************************************************************/
PRIVATE VOID null_msg(MSG* msg)
{
    MSG temp_msg; /* create a special msg for "no msg" */
    /* from itself to itself */
    temp_msg.sproc_id = current_proc->id;
    temp_msg.dproc_id = current_proc->id;
    temp_msg.sig = SPE_NOMSG; /* sig number for "no msg" */
    temp_msg.datasize = NULL;
    temp_msg.data = NULL; /* no data */
    /* copy the msg to receive space */
    ict_memcpy ( ( POINTER ) &temp_msg, ( POINTER ) msg, sizeof ( MSG ) );
}
