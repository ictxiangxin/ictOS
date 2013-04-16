;==============================================================================;
;                      ictOS Contents of protected mode                        ;
;                                                                      by: ict ;
;==============================================================================;

DAT_r           equ     0000000000000000b
DAT_r_a         equ     0000000000000001b
DAT_rw          equ     0000000000000010b
DAT_rw_a        equ     0000000000000011b
DAT_r_e         equ     0000000000000100b
DAT_r_e_a       equ     0000000000000101b
DAT_rw_e        equ     0000000000000110b
DAT_rw_e_a      equ     0000000000000111b
DAT_x           equ     0000000000001000b
DAT_x_a         equ     0000000000001001b
DAT_xr          equ     0000000000001010b
DAT_xr_a        equ     0000000000001011b
DAT_x_c         equ     0000000000001100b
DAT_x_c_a       equ     0000000000001101b
DAT_xr_c        equ     0000000000001110b
DAT_xr_c_a      equ     0000000000001111b
DAT_e286tss     equ     0000000000000001b
DAT_ldt         equ     0000000000000010b
DAT_b286tss     equ     0000000000000011b
DAT_286call     equ     0000000000000100b
DAT_task        equ     0000000000000101b
DAT_286int      equ     0000000000000110b
DAT_286trap     equ     0000000000000111b
DAT_e386tss     equ     0000000000001001b
DAT_b386tss     equ     0000000000001011b
DAT_386call     equ     0000000000001100b
DAT_386int      equ     0000000000001110b
DAT_386trap     equ     0000000000001111b
DAT_ring_0      equ     0000000000000000b
DAT_ring_1      equ     0000000000100000b
DAT_ring_2      equ     0000000001000000b
DAT_ring_3      equ     0000000001100000b
DAT_present     equ     0000000010000000b
DAT_nopresent   equ     0000000000000000b
DAT_datacode    equ     0000000000010000b
DAT_systemgate  equ     0000000000000000b
DAT_g_1b        equ     0000000000000000b
DAT_g_4kb       equ     1000000000000000b
DAT_32a32i      equ     0100000000000000b
DAT_16a16i      equ     0000000000000000b
DAT_segment4gb  equ     0100000000000000b
DAT_segment64kb equ     0000000000000000b
DAT_esp         equ     0100000000000000b
DAT_sp          equ     0000000000000000b
DAT_80386igate  equ     1010111000000000b

PG_present      equ     0000000000000001b
PG_nopresent    equ     0000000000000000b
PG_r            equ     0000000000000000b
PG_rw           equ     0000000000000010b
PG_spl          equ     0000000000000000b
PG_upl          equ     0000000000000100b
PG_wb           equ     0000000000000000b
PG_wt           equ     0000000000001000b
PG_cache        equ     0000000000000000b
PG_nocache      equ     0000000000010000b
PG_4kb          equ     0000000000000000b

DescSize        equ     0x8
LDTElemSum      equ     0x4
