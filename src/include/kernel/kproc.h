/*================================================================================*/
/*                      ictOS kernel proc managment contents                      */
/*                                                                        by: ict */
/*================================================================================*/
#ifndef _KPROC_H_
#define _KPROC_H_

#define PID_KPM             0x0
#define PID_MEM             0x1
#define PID_KB              0x2
#define PID_HDD             0x3
#define PID_VD              0x4

#define PRIV_KPM            0x0
#define PRIV_MEM            0x10
#define PRIV_KB             0x4
#define PRIV_HDD            0x2
#define PRIV_KFS            0x2
#define PRIV_VD             0x8

#define KPS_OK              0x0
#define KPS_WAITMSG         0x1
#define KPS_WAITINT         0x2
#define KPS_SLEEP           0x3

#define KP_CODE_BASE        0x0
#define KP_CODE_LIMIT       0xfffff
#define KP_DATA_BASE        0x0
#define KP_DATA_LIMIT       0xfffff
#define KP_STACK_BASE       0x0
#define KP_STACK_LIMIT      0xfffff
#define KP_VIDEO_BASE       0xb8000
#define KP_VIDEO_LIMIT      0x7fff

#define KP_CODE_LD_INDEX    0x0
#define KP_DATA_LD_INDEX    0x1
#define KP_STACK_LD_INDEX   0x2
#define KP_VIDEO_LD_INDEX   0x3

#define SELECTOR_LDT        0X4
#define SLT_RING_0          0x0
#define SLT_RING_1          0x1
#define SLT_RING_2          0x2
#define SLT_RING_3          0x3

#define DESC_ATTR_LDT       0x0082
#define DA_PCODE            0xc0b8 /* 1100000010111000 */
#define DA_PDATA            0x80b2
#define DA_PSTACK           0x80b2
#define DA_PVIDEO           0x00b2

#define KPROC_CODE_SLT      0x0000 | SELECTOR_LDT | SLT_RING_1
#define KPROC_DATA_SLT      0x0008 | SELECTOR_LDT | SLT_RING_1
#define KPROC_STACK_SLT     0x0010 | SELECTOR_LDT | SLT_RING_1
#define KPROC_VIDEO_SLT     0x0018 | SELECTOR_LDT | SLT_RING_1

#endif
