/*================================================================================*/
/*                          ictOS kernel Main Function                            */
/*                                                                        by: ict */
/*================================================================================*/

#include "kasm.h"
#include "servers.h"
#include "public.h"
#include "constent.h"
#include "type.h"
#include "kproc.h"
#include "sig.h"
#include "daemon.h"
#include "../io/i8259a.h"
#include "../io/BIOScolor.h"

void testb()
{
    ict_cprintf ( COLOR_WHITE, "pid:%d is running ...\n", ict_mypid() );
    MSG m;
    BYTE buff[512] = "1";
    WORD ln[15] = {'k', 'e', 'r', 'n', 'e', 'l', '.', 'i', 'c', 't'};
    //WORD ln[15] = {'t', 'e', 's', 't'};
    ict_cprintf( COLOR_RED, "open file TEST/TEST.ICT : <%d>\n", ict_open_sname("TEST/TEST.TXT", 1));
    ict_cprintf( COLOR_RED, "open file kernel.ict : <%d>\n", ict_open_lname(ln, 1));
    ict_seek(1, 3, SEEK_START);
    ict_write(1, 512, buff);
    ict_printf("{%s}", buff);
    while ( TRUE )
    {
        send_msg (PID_KB, KB_KEY, NULL, NULL);
        recv_msg ( &m );
        ict_cprintf(COLOR_YELLOW, "\"%d\"", m.sig);
        //for ( i = 0; i < 1000000; i++ );
    }
}

void testc()
{
    //ict_sleep();
    int i = 0;
    char c = '0';
    char color = COLOR_BLUE;
    int x = 0;
    int y = 60;
    MSG m;
    ict_cprintf ( COLOR_WHITE, "pid:%d is running ...\n", ict_mypid() );
    send_msg ( PID_KB, KB_KEYS, NULL, NULL );
    while ( TRUE )
    {
        read_msg ( &m );
        if ( m.sig == 17 )
            x = x == 0 ? 0 : x - 1;
        if ( m.sig == 30 )
            y = y == 0 ? 0 : y - 1;
        if ( m.sig == 31 )
            x = x == 24 ? 24 : x + 1;
        if ( m.sig == 32 )
            y = y == 79 ? 79 : y + 1;
        //void (*f)();
        //f = 0xffffffff;
        //(*f)();
        call_dropchar ( c, color, x, y );
        call_dropchar ( c, color, x, y - 1 );
        call_dropchar ( c, color, x, y + 1 );
        color = ++color % 0xf + 1;
        c++;
        if ( c > '9' )
            c = '0';
        if ( m.sig )
            continue;
        for ( i = 0; i < 1000000; i++ );
    }
}

void testd()
{
    //ict_sleep();
    int i = 0;
    int s = 0;
    MSG m;
    ict_cprintf ( COLOR_WHITE, "pid:%d is running ...\n", ict_mypid() );
    while ( TRUE )
    {
        if ( send_msg ( 9, s, NULL, NULL ))
            s++;
        for ( i = 0; i < 1000000; i++ );
    }
}

void teste()
{
    //ict_sleep();
    int i = 0;
    MSG m;
    ict_cprintf ( COLOR_WHITE, "pid:%d is running ...\n", ict_mypid() );
    while ( TRUE )
    {
        recv_msg ( &m );
        ict_cprintf ( COLOR_MAGENTA, "_%d_", m.sig );
        for ( i = 0; i < 1000000; i++ );
    }
}

void testf()
{
    int i;
    int p = 0;
    char sb[] = {'-', '\\', '|', '/'};
    ict_cprintf ( COLOR_WHITE, "pid:%d is running ...\n", ict_mypid() );
    while ( TRUE )
    {
        msg_dropchar ( sb[p], COLOR_LIGHTGREEN, 11, 70 );
        msg_dropchar ( sb[p], COLOR_LIGHTGREEN, 9, 70 );
        msg_dropchar ( sb[p], COLOR_LIGHTGREEN, 10, 71 );
        msg_dropchar ( sb[p], COLOR_LIGHTGREEN, 10, 69 );
        msg_dropchar ( sb[p], COLOR_LIGHTGREEN, 12, 70 );
        msg_dropchar ( sb[p], COLOR_LIGHTGREEN, 8, 70 );
        msg_dropchar ( sb[p], COLOR_LIGHTGREEN, 10, 72 );
        msg_dropchar ( sb[p], COLOR_LIGHTGREEN, 10, 68 );
        p = ++p % 4;
        if(p == 0)
            ict_cprintf(COLOR_GREEN, "[0x%x]", ict_idlesize());
        for ( i = 0; i < 1000000; i++ );
    }
}

void create_service_deamon()
{
    add_kernelproc ( kpm_daemon, PRIV_KPM ); /* kpid: 0 */
    add_kernelproc ( mem_daemon, PRIV_MEM ); /* kpid: 1 */
    add_kernelproc ( keyboard_daemon, PRIV_KB ); /* kpid: 2 */
    add_kernelproc ( hd_daemon, PRIV_HD ); /* kpid: 3 */
    add_kernelproc ( video_daemon, PRIV_VD ); /* kpid: 4 */
    add_kernelproc ( kfs_daemon, PRIV_KFS ); /* kpid: 5 */
}

PUBLIC KPROC* current_proc;

/******************************************************************/
/* the main function of kernel                                    */
/******************************************************************/
PUBLIC VOID kernel()
{
    init_video(); /* init video server */
    init_mem();   /* init memory management */
    init_msg();   /* init msg service */
    init_clock(); /* init clock service */
    init_kb();    /* init keyboard service */
    init_hd();    /* init hard disk service */
    init_kfs();   /* init kernel file system service */
    create_service_deamon(); /* create system service daemon */
    add_kernelproc ( testb, 5 );
    add_kernelproc ( testc, 2 );
    add_kernelproc ( testd, 3 );
    add_kernelproc ( teste, 2 );
    add_kernelproc ( testf, 4 );
    ict_cli();
    ict_out ( SLAVE_8259A_OCW1, IRQ_ATA );
    ict_out ( MASTER_8259A_OCW1, IRQ_KEYBOARD & IRQ_CLOCK & IRQ_SLAVE8259A );   /* open the i8259a interrupts */
    ict_loadLDT ( current_proc->id + RESERVED_DESC_BY_KERNEL );
    ict_execute ( current_proc );
}
