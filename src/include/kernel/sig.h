/*================================================================================*/
/*                              ictOS signals of msg                              */
/*                                                                        by: ict */
/*================================================================================*/

#ifndef _SIG_H_
#define _SIG_H_

/******************************************************************/
/* special signals                                                */
/******************************************************************/
#define SPE_NOMSG   0x0000 /* no msg in msg buf */

/******************************************************************/
/* signals of kproc mannager                                      */
/******************************************************************/
#define KPM_INIT    0x0000 /* tell kpm init all procs */

#define KPM_HAVEMSG  0x0100 /* one proc have msg */
#define KPM_HAVEINT  0x0200 /* one proc have int for it */
#define KPM_UPPRIV   0x0300 /* increase the priv of the proc */
#define KPM_DOWNPRIV 0x0400 /* decrease the priv of the proc */
#define KPM_WAKEUP   0x0500 /* wake up a proc */

#define KPM_WAITMSG 0x0001 /* proc will wait until receive a msg */
#define KPM_WAITINT 0x0002 /* proc will wait a int */
#define KPM_SLEEP   0x0003 /* proc will sleep until other proc aweak it */
#define KPM_HUNG    0x0004 /* proc is hung, and increase its hung value */
#define KPM_FULL    0x0005 /* proc is full, and set its hung value as 0 */

/******************************************************************/
/* signals of mem daemon                                          */
/******************************************************************/
#define MEM_ANS    0x0001 /* answer to one msg */
#define MEM_MALLOC 0x0002 /* alloc memory */
#define MEM_FREE   0x0003 /* free memory */
#define MEM_IDLE   0x0004 /* idle memory size */

/******************************************************************/
/* signals of hard disk daemon                                    */
/******************************************************************/
#define HD_READ      0x0001 /* request read data from disk to buff */
#define HD_WRITE     0x0002 /* request write data from buff to disk */
#define HD_READOVER  0x0003 /* read over */
#define HD_WRITEOVER 0x0004 /* write over */

/******************************************************************/
/* signals of kernel file system                                  */
/******************************************************************/
#define KFS_OPEN    0x0001 /* kproc open a file */

/******************************************************************/
/* signals of video service                                       */
/******************************************************************/
#define VD_CHAR       0x0001
#define VD_STRING     0x0002
#define VD_DROPCHAR   0x0003
#define VD_TEXTMODE   0x0004
#define VD_GRAPHMODE  0x0005

/******************************************************************/
/* signals of keyboard daemon                                     */
/******************************************************************/
#define KB_KEY      0x0000 /* request a key */
#define KB_KEYS     0x0001 /* request a group of keys */
#define KB_KEYS_END 0x0002 /* the end of group key request */

#endif
