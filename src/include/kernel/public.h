/*================================================================================*/
/*                         ictOS Public Structure Header                          */
/*                                                                        by: ict */
/*================================================================================*/

#ifndef _PUBLIC_H_
#define _PUBLIC_H_

#include "type.h"
#include "constent.h"

/* Segment Descriptor */
typedef struct SegDesc
{
    WORD seg_limit_15_0;
    WORD seg_base_15_0;
    BYTE seg_base_23_16;
    BYTE seg_attr_low;
    BYTE seg_attr_high;
    BYTE seg_base_31_24;
} SEGDESC;

/* Gate Descriptor */
typedef struct GateDesc
{
    WORD gate_offset_15_0;
    WORD gate_selector;
    WORD gate_attr;
    WORD gate_offset_31_16;
} GATEDESC;

/* Local Descriptor Table */
typedef struct ldt
{
    SEGDESC ld[LDT_ELEMSUM];
} LDT;

/* TSS */
typedef struct tss
{
    DWORD back;
    DWORD esp0;
    DWORD ss0;
    DWORD esp1;
    DWORD ss1;
    DWORD esp2;
    DWORD ss2;
    DWORD cr3;
    DWORD eip;
    DWORD flags;
    DWORD eax;
    DWORD ecx;
    DWORD edx;
    DWORD ebx;
    DWORD esp;
    DWORD ebp;
    DWORD esi;
    DWORD edi;
    DWORD es;
    DWORD cs;
    DWORD ss;
    DWORD ds;
    DWORD fs;
    DWORD gs;
    DWORD ldt;
    WORD  trap;
    WORD  iomapbase;
} TSS;

/* Kernel Processor Message */
typedef struct msg
{
    DWORD sproc_id;
    DWORD dproc_id;
    DWORD sig;
    DWORD datasize;
    DWORD data; /* if the datasize != 0, this value is a pointer of data */
} MSG;

/* message buffer */
typedef struct msgbuf
{
    DWORD hook; /* the id of this buf hooked proc */
    DWORD read_p; /* the msg read pointer, it point to the first msg */
    DWORD write_p; /* the msg write pointer, it point to the first empty msg */
    MSG msglist[MSGBUF_SIZE]; /* msg list */
    struct msgbuf* next;    /* next buf, which hooked on this proc */
} MSGBUF;

/* message pool */
typedef struct msgpool
{
    MSGBUF* idle; /* idle buf address */
    MSGBUF  msgbuflist[MSGBUF_SUM]; /* buf pool */
    struct  msgpool* back; /* back msg pool */
    struct  msgpool* next; /* next msg pool */
} MSGPOOL;

/* memory block */
typedef struct memblock
{
    POINTER addr; /* address of this memory block */
    DWORD   size; /* size of this memory block */
    struct memblock* back; /* backward memory block */
    struct memblock* next; /* forward memory block */
} MEMBLOCK;

typedef struct memrecord
{
    DWORD    count;
    DWORD    statmap;
    MEMBLOCK spacelist[MEMRECORD_SPACESUM];
    struct memrecord* back;
    struct memrecord* next;
} MEMRECORD;

/* Kernel PCB */
typedef struct kproc
{
    DWORD fs;
    DWORD gs;
    DWORD es;
    DWORD ds;
    DWORD edi;
    DWORD esi;
    DWORD ebp;
    DWORD vesp;
    DWORD ebx;
    DWORD edx;
    DWORD ecx;
    DWORD eax;
    DWORD eip;
    DWORD cs;
    DWORD eflags;
    DWORD esp;
    DWORD ss;

    LDT procldt;

    BYTE id;
    BYTE present;
    BYTE state;

    MSGBUF* msgentry;
    BYTE    msglock;
    BYTE    havemsg;
    BYTE    haveint;

    DWORD priv;
    DWORD count;
    DWORD hung;

    struct kproc* next;
} KPROC;

/* Kernel processor list */
typedef struct kproclist
{
    KPROC procs[KPROC_SUM];
    DWORD proc_sum;
} KPROCLIST;

/* Keyboard Buffer Node */
typedef struct KeyboardNode
{
    struct KeyboardNode* next;
    struct KeyboardNode* back;
    BYTE scan_code;
} KEYBOARDNODE;

/* Keyboard Buffer */
typedef struct KeyboardBuffer
{
    KEYBOARDNODE* free_node;
    KEYBOARDNODE* queue_entry;
    KEYBOARDNODE buffer_node[KEYBOARD_BUFFER_SIZE];
    DWORD used_size;
} KEYBOARDBUFFER;

/* ATA Date Struct */
typedef struct ATAdata
{
    DWORD secnum; /* sector number */
    DWORD seccnt; /* sector count */
    DWORD dev; /* device */
    BYTE* buff; /* buffer of data */
} ATADATA;

#endif

