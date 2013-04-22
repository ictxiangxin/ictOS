/*================================================================================*/
/*                        ICT Perfect 2.00 Descriptor init                        */
/*                                                                        by: ict */
/*================================================================================*/

#include "public.h"
#include "type.h"

PUBLIC VOID ict_Descinit(SEGDESC* sd, DWORD base, DWORD limit, WORD attr)
{
    sd->seg_limit_15_0 = (WORD)(limit & 0xffff);
    sd->seg_base_15_0  = (WORD)(base & 0xffff);
    sd->seg_base_23_16 = (BYTE)((base >> 0x10) & 0xff);
    sd->seg_attr_low   = (BYTE)(attr);
    sd->seg_attr_high  = (BYTE)((attr >> 0x8) | (limit >> 0x10) & 0xf);
    sd->seg_base_31_24 = (BYTE)((base >> 0x18) & 0xff);
}
