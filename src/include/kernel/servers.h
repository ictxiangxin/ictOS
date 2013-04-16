/*================================================================================*/
/*                         ictOS Servers functions Header                         */
/*                                                                        by: ict */
/*================================================================================*/

#include "type.h"
#include "public.h"

#ifndef _SERVERS_H_
#define _SERVERS_H_

PUBLIC VOID init_keyboard();
PUBLIC BYTE get_key();

/* clock service */
PUBLIC VOID init_clock();

/* hard disk service */
PUBLIC VOID init_hd();

/* memory management */
PUBLIC VOID    init_mem();
PUBLIC VOID    mem_daemon();
PUBLIC POINTER ict_malloc(DWORD size);
PUBLIC DWORD   ict_free(POINTER addr);
PUBLIC POINTER call_malloc(DWORD size);
PUBLIC DWORD   call_free(POINTER addr);
PUBLIC POINTER msg_malloc(DWORD size);
PUBLIC DWORD   msg_free(POINTER addr);
PUBLIC DWORD   ict_idlesize();

/* kernel proc management */
PUBLIC VOID    init_kproc();
PUBLIC VOID    kpm_daemon();
PUBLIC VOID    add_kernelproc(POINTER func, DWORD privilege);
PUBLIC VOID    ict_sleep();
PUBLIC VOID    ict_wakeup(DWORD kpid);
PUBLIC VOID    ict_waitint();
PUBLIC VOID    ict_intfor(DWORD kpid);
PUBLIC VOID    ict_hung();
PUBLIC VOID    ict_full();
PUBLIC DWORD   ict_mypid();

/* msg service */
PUBLIC VOID    init_msgpool();
PUBLIC VOID    msgbuf_hook(DWORD proc_id);
PUBLIC DWORD   send_msg(DWORD dest_proc_id, DWORD sig, DWORD data, DWORD datasize);
PUBLIC DWORD   read_msg(MSG* msg);
PUBLIC VOID    recv_msg(MSG* msg);
PUBLIC VOID    clear_msg();
PUBLIC BYTE    have_msg();
PUBLIC BYTE    have_int();

/* video service */
PUBLIC VOID    init_video();
PUBLIC VOID    video_daemon();
PUBLIC VOID    ict_putchar(BYTE c);
PUBLIC VOID    ict_cputchar(BYTE c, BYTE color);
PUBLIC VOID    ict_dropchar(BYTE c, BYTE color, DWORD x, DWORD y);
PUBLIC VOID    ict_printf(BYTE* format, ...);
PUBLIC VOID    ict_cprintf(BYTE color, BYTE* format, ...);

#endif
