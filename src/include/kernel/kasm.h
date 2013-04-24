/*================================================================================*/
/*                          ictOS Asm Functions Header                            */
/*                                                                        by: ict */
/*================================================================================*/

#include "type.h"

#ifndef _BASICFUNC_H_
#define _BASICFUNC_H_

PUBLIC VOID  ict_out(DWORD port, BYTE data);	/* send byte to port */
PUBLIC BYTE  ict_in(DWORD port);	/* receive byte from port */
PUBLIC BYTE  ict_cli();	/* clear interrupt flag */
PUBLIC BYTE  ict_sti();	/* set interrupt flag */
PUBLIC DWORD ict_lock(DWORD* lock);
PUBLIC VOID  ict_unlock(DWORD* lock);
PUBLIC VOID  ict_memcpy(POINTER source, POINTER destination, DWORD size);
PUBLIC DWORD ict_cursorlocation();
PUBLIC VOID  ict_setcursor(DWORD loc);
PUBLIC DWORD ict_startlocation();
PUBLIC VOID  ict_setstart(DWORD loc);
PUBLIC VOID  ict_putc(DWORD data, DWORD cursorlocation);
PUBLIC VOID  ict_refreshvideo(DWORD startlocation, DWORD video_width, DWORD video_high, DWORD buffsize);
PUBLIC VOID  ict_setupint(DWORD int_number, POINTER func); /* setup a interrupt */
PUBLIC VOID  ict_loadLDT(DWORD GD_number);	/* load LDT and set ldtr */
PUBLIC VOID  ict_loadGD(POINTER gd_temp, DWORD number);	/* load Globla Descriptor to GDT */
PUBLIC VOID  ict_execut(POINTER proc);
PUBLIC VOID  ict_done();

#endif
