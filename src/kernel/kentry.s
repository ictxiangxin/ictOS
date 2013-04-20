;==============================================================================;
;                             ictOS kernel entry                               ;
;                                                                      by: ict ;
;==============================================================================;

extern kernel

global _start

[bits 32]
_start:
    call    kernel  ; jmp to kernel c part
    jmp     $
