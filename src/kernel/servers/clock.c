/*================================================================================*/
/*                         ICT Perfect 2.00 clock works                           */
/*                                                                        by: ict */
/*================================================================================*/

#include "type.h"
#include "public.h"
#include "kasm.h"
#include "constent.h"
#include "kproc.h"
#include "servers.h"
#include "../io/i8254.h"
#include "../io/BIOScolor.h"

PUBLIC VOID clock_interrupt();

PUBLIC KPROCLIST kernelproclist; /* kernel proc list */
PUBLIC KPROC* current_proc; /* init the current proc is the first proc of proc list */
PUBLIC KPROC* kpm;

/******************************************************************/
/* init clock server, set the clock frequency by set the i8254    */
/******************************************************************/
PUBLIC VOID init_clock()
{
    //ict_printf ( COLOR_YELLOW, "Setup clock service...          " );
    DWORD t = 30; /* 0.001s */
    t = I8254_FREQUENCY * t / 1000; /* translate to the count number */
    /* set the i8254 count number system, work mode, and read/write mode */
    ict_out ( I8254_MCR, I8254_BINARY | I8254_RG | I8254_LOWHIGH );
    ict_out ( I8254_COUNTER0, ( BYTE ) t ); /* write the low byte */
    ict_out ( I8254_COUNTER0, ( BYTE ) ( t >> 0x8 ) ); /* write the high byte */
    ict_setupint ( 0x20, clock_interrupt ); /* setup the clock interrupt handle */
    //ict_printf ( COLOR_YELLOW, "[ OK ]\n" );
}

/******************************************************************/
/* clock interrupt handle, and the kproc scheduler                */
/******************************************************************/
PUBLIC VOID int_clock ( POINTER regs )
{
    ict_memcpy ( &regs, current_proc, 68 ); /* store all regs of proc */
    if ( kpm->havemsg )
    {
        current_proc = kpm; /* make it become current_proc, for next clock interrupt, we can make it right */
        ict_loadLDT ( current_proc->id + RESERVED_DESC_BY_KERNEL ); /* change the LDT to this proc */
        ict_execute ( current_proc ); /* execute this proc */
    }
    while ( TRUE ) /* actually, here only loop 2 times at most */
    {
        KPROC* s_p = current_proc; /* scheduled proc */
        KPROC* t_p = current_proc; /* temp proc, for ergodic the proc list */
        do /* this loop is to choose the best proc */
        {
            t_p = t_p->next; /* test the next proc */
            if ( t_p->status != KPS_OK )
                continue;
            /* when one proc's final privilege is bigger than the scheduled proc,
               replace the old one.and if one proc's final privilege is equal to
               the scheduled proc, compare the proc count to decided which one
               should be scheduled(choose the count bigger one) */
            if (
                t_p->count * t_p->hung > s_p->count * s_p->hung
                ||
                (
                    t_p->count * t_p->hung == s_p->count * s_p->hung
                    &&
                    t_p->count > s_p->count
                )
            )
                s_p = t_p; /* replace the scheduled proc */
        }
        while ( t_p != current_proc ); /* the end condition is loop to the linklist head */
        t_p = current_proc; /* init the temp proc, for next loop */
        /* this loop is to change proc's hung value,
           some proc not be scheduled in this time,
           they may become more hungry.
           and the scheduled proc become full */
        do
        {
            t_p = t_p->next; /* change the next proc's hungry status */
            if ( t_p != s_p ) /* if this proc is not scheduled proc */
                if ( t_p->status == KPS_OK )
                    t_p->hung++; /* make it more hungry */
                else /* so, this is the scheduled proc */
                    t_p->hung = 1; /* it is full now */
        }
        while ( t_p != current_proc ); /* the end condition is loop to the linklist head */
        /* when the best proc's count is 0, it means
           all proc's count is 0, so, we should reset
           all count of them, make is works like before.
           and it means one schedule block is over */
        if ( s_p->count == 0 )
        {
            t_p = current_proc; /* init the temp proc, for next loop */
            do
            {
                t_p = t_p->next; /* reset next proc's count */
                t_p->count = t_p->priv; /* the count is set to the proc's privilege */
            }
            while ( t_p != current_proc ); /* the end condition is loop to the linklist head */
            continue; /* all count are reseted, so we should restart this scheduler programme */
        }
        s_p->count--; /* finally, this proc is considered the best scheduled proc, so we decrease its count */
        current_proc = s_p; /* make it become current_proc, for next clock interrupt, we can make it right */
        ict_loadLDT ( current_proc->id + RESERVED_DESC_BY_KERNEL ); /* change the LDT to this proc */
        ict_execute ( current_proc ); /* execute this proc */
    }
}
