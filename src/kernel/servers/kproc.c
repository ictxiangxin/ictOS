/*================================================================================*/
/*                            ictOS kernel proc managment                         */
/*                                                                        by: ict */
/*================================================================================*/

#include "kasm.h"
#include "constent.h"
#include "type.h"
#include "public.h"
#include "kproc.h"
#include "servers.h"
#include "sig.h"
#include "../io/BIOScolor.h"

PRIVATE VOID kproc_manager();

KPROCLIST kernelproclist;	/* kernel proc list */
KPROC* current_proc = & ( kernelproclist.procs[PID_KPM] );	/* store the pointer of current running proc */
KPROC* kpm = & ( kernelproclist.procs[PID_KPM] );	/* store the pointer of kpm */

/******************************************************************/
/* add a new kproc to kproc list                                  */
/******************************************************************/
PUBLIC VOID add_kernelproc ( POINTER func, DWORD privilege )	/* load a kernel proc to kproc list */
{
    if ( kernelproclist.proc_sum == KPROC_SUM )	/* if the proc list is full */
    {
        ict_cprintf ( COLOR_RED, "\nProcess List Full !!!\n" );	/* print the error msg and do nothing */
        return;
    }
    SEGDESC ldtgd;	/* descriptor of ldt */
    SEGDESC ld_code;	/* code seg local descriptor */
    SEGDESC ld_data;	/* data seg local descriptor */
    SEGDESC ld_stack;	/* stack seg local descriptor */
    SEGDESC ld_video;	/* video seg local descriptor */
    DWORD proc_number = kernelproclist.proc_sum;	/* get the proc sum */
    LDT* ldt_addr = & ( kernelproclist.procs[proc_number].procldt );	/* get the addr of ldt */
    KPROC* this = & ( kernelproclist.procs[proc_number] );	/* get the addr of this proc info */
    ict_Descinit ( &ldtgd, ldt_addr, LDT_ELEMSUM * DESC_SIZE, DESC_ATTR_LDT );	/* init the global descriptor of ldt */
    ict_loadGD ( &ldtgd, proc_number + RESERVED_DESC_BY_KERNEL );	/* load the global descriptor of ldt to gdt */
    ict_loadLDT ( proc_number + RESERVED_DESC_BY_KERNEL );	/* load the selector to ldtr */
    ict_Descinit ( &ld_code, KP_CODE_BASE, KP_CODE_LIMIT, DA_PCODE );	/* init code descriptor */
    ict_Descinit ( &ld_data, KP_DATA_BASE, KP_DATA_LIMIT, DA_PDATA );	/* init data descriptor */
    ict_Descinit ( &ld_stack, KP_STACK_BASE, KP_STACK_LIMIT, DA_PSTACK );	/* init stack descriptor */
    ict_Descinit ( &ld_video, KP_VIDEO_BASE, KP_VIDEO_LIMIT, DA_PVIDEO );	/* init video descriptor */
    ict_memcpy ( &ld_code, & ( ldt_addr->ld[KP_CODE_LD_INDEX] ), DESC_SIZE );	/* load the code descriptor */
    ict_memcpy ( &ld_data, & ( ldt_addr->ld[KP_DATA_LD_INDEX] ), DESC_SIZE );	/* load the data descriptor */
    ict_memcpy ( &ld_stack, & ( ldt_addr->ld[KP_STACK_LD_INDEX] ), DESC_SIZE );	/* load the stack descriptor */
    ict_memcpy ( &ld_video, & ( ldt_addr->ld[KP_VIDEO_LD_INDEX] ), DESC_SIZE );	/* load the video descriptor */
    this->fs = KPROC_DATA_SLT;	/* set fs as data selector */
    this->gs = KPROC_VIDEO_SLT;	/* set gs as video selector */
    this->es = KPROC_DATA_SLT;	/* set es as data selector */
    this->ds = KPROC_DATA_SLT;	/* set ds as data selector */
    this->cs = KPROC_CODE_SLT;	/* set cs as code selector */
    this->ss = KPROC_STACK_SLT;	/* set ss as stack selector */
    this->eip = ( DWORD ) func;	/* set eip point to the proc func start */
    this->esp = KPROC_STACK_BASE + KPROC_STACK_SIZE * ( proc_number + 1 );	/* set the stack pointer */
    this->eflags = 0x1202;	/* eflags of the proc */
    this->status = KPS_OK;	/* all procs will be wakeuped by kproc manager */
    this->statuslock = FALSE;
    this->priv = privilege;	/* set the privilege of this proc */
    this->count = this->priv;	/* set the count of the proc */
    this->hung = 1;	/* set the hungry of the proc, 1 means it is full */
    if ( proc_number != 0 )	/*  if this proc is not first proc in proc list */
        kernelproclist.procs[proc_number - 1].next = this;	/* let front proc link to this */
    this->next = & ( kernelproclist.procs[0] );	/* now this proc is end of the proc list, it must point to start */
    this->msglock = FALSE;
    this->havemsg = FALSE;
    this->haveint = FALSE;
    this->present = 1;	/* now, all load works are completed, so this proc is present */
    this->id = proc_number;	/* set the id of the proc */
    this->msgentry = NULL;	/* prepare hook buf to this proc */
    msgbuf_hook ( this->id );
    kernelproclist.proc_sum++;	/* proc sum increase */
}

/******************************************************************/
/* kproc mannager, handle all kpm sig to manage kprocs            */
/******************************************************************/
PUBLIC VOID kpm_daemon()
{
    MSG msg;	/* temp msg space */
    while ( TRUE )	/* msg loop */
    {
        while ( !have_msg() )	/* if no msg in buf */
            ict_done();	/* no more woks */
        send_msg(PID_KPM, SPE_NOMSG, NULL, NULL);
        if ( !read_msg ( &msg ) )	/* read the msg */
            continue;	/* if read fail, wait until have msg */
        /* kpm have the top privilege, so needn't lock anything */
        switch ( msg.sig )	/* handle each sig, which at the high 2-byte */
        {
            case KPM_HAVEMSG :	/* proc has msg */
                if ( kernelproclist.procs[msg.data].status == KPS_WAITMSG )
                    kernelproclist.procs[msg.data].status = KPS_OK;
                break;
            case KPM_HAVEINT :	/* proc has int, and if the proc is OK, we must record this int */
                if ( kernelproclist.procs[msg.data].status == KPS_WAITINT )
                    kernelproclist.procs[msg.data].status = KPS_OK;
                else	/* the proc is not waiting the int */
                    kernelproclist.procs[msg.data].haveint = TRUE;	/* record this int */
                break;
            case KPM_UPPRIV :	/* increase the priv of the proc */
                kernelproclist.procs[msg.data].priv++;
                break;
            case KPM_DOWNPRIV :	/* decrease the priv of the proc */
                kernelproclist.procs[msg.data].priv--;
                break;
            case KPM_WAKEUP :	/* wake up a proc */
                if ( kernelproclist.procs[msg.data].status == KPS_SLEEP )
                    kernelproclist.procs[msg.data].status = KPS_OK;
                break;
            case KPM_WAITMSG :	/* I will wait the msg */
                if ( kernelproclist.procs[msg.sproc_id].havemsg == FALSE )
                    kernelproclist.procs[msg.sproc_id].status = KPS_WAITMSG;
                break;
            case KPM_WAITINT :	/* I will wait the int */
                if ( kernelproclist.procs[msg.sproc_id].haveint == FALSE )
                    kernelproclist.procs[msg.sproc_id].status = KPS_WAITINT;
                else	/* the proc have int before, so we clean this record and proc needn't wait */
                    kernelproclist.procs[msg.sproc_id].haveint = FALSE;
                break;
            case KPM_SLEEP :	/* I will sleep */
                kernelproclist.procs[msg.sproc_id].status = KPS_SLEEP;
                break;
            case KPM_HUNG :	/* I am hungry, need more time to execute */
                kernelproclist.procs[msg.sproc_id].hung++;
                break;
            case KPM_FULL :	/* I am full, and needn't too much time */
                kernelproclist.procs[msg.sproc_id].hung = 0;
                break;
        }
        dest_msg ( &msg );
    }
}

/******************************************************************/
/* proc sleep until some procs wake up it                         */
/******************************************************************/
PUBLIC VOID ict_sleep()
{
    if(!ict_lock(&(kernelproclist.procs[current_proc->id].statuslock)))
    {
        kernelproclist.procs[current_proc->id].status = KPS_SLEEP;
        ict_unlock(&(kernelproclist.procs[current_proc->id].statuslock));
    }
    else
        send_msg ( PID_KPM, KPM_SLEEP, NULL, NULL );	/* send a sleep sig to kpm */
    ict_done();	/* nothing to do */
}

/******************************************************************/
/* let kpm wake up the proc                                       */
/******************************************************************/
PUBLIC VOID ict_wakeup ( DWORD kpid )
{
    if(kernelproclist.procs[kpid].status == KPS_SLEEP)
        if(!ict_lock(&(kernelproclist.procs[kpid].statuslock)))
        {
            kernelproclist.procs[kpid].status = KPS_OK;
            ict_unlock(&(kernelproclist.procs[kpid].statuslock));
        }
        else
            send_msg ( PID_KPM, KPM_WAKEUP, kpid, NULL );	/* send the wakeup msg to kpm */
}

/******************************************************************/
/* let kpm wake up the proc                                       */
/******************************************************************/
PUBLIC VOID ict_waitint()
{
    if ( !have_int() )
    {
        if(!ict_lock(&(kernelproclist.procs[current_proc->id].statuslock)))
        {
            kernelproclist.procs[current_proc->id].status = KPS_WAITINT;
            ict_unlock(&(kernelproclist.procs[current_proc->id].statuslock));
        }
        else
            send_msg ( PID_KPM, KPM_WAITINT, NULL, NULL );	/* send a wait int sig to kpm */
        ict_done();	/* nothing to do */
    }
}

/******************************************************************/
/* let kpm wake up the proc                                       */
/******************************************************************/
PUBLIC VOID ict_intfor ( DWORD kpid )
{
    if(kernelproclist.procs[kpid].haveint == FALSE)
        if(!ict_lock(&(kernelproclist.procs[kpid].statuslock)))
        {
            kernelproclist.procs[kpid].status = KPS_OK;
            ict_unlock(&(kernelproclist.procs[kpid].statuslock));
        }
        else
            send_msg ( PID_KPM, KPM_HAVEINT, kpid, NULL );	/* tell the kpm that the proc have int */
}

/******************************************************************/
/* proc hung, need more time to execute                           */
/******************************************************************/
PUBLIC VOID ict_hung()
{
    send_msg ( PID_KPM, KPM_HUNG, NULL, NULL );	/* just send a hungry sig to kpm */
}

/******************************************************************/
/* proc full, can give more time to others                        */
/******************************************************************/
PUBLIC VOID ict_full()
{
    send_msg ( PID_KPM, KPM_FULL, NULL, NULL );	/* send a full sig to kpm */
    ict_done();	/* because it is full, so nothing to do */
}

/******************************************************************/
/* return the pid of this proc                                    */
/******************************************************************/
PUBLIC DWORD ict_mypid()
{
    return current_proc->id;
}
