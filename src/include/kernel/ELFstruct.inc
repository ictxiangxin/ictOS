;==============================================================================;
;                          ictOS ELF file structure                            ;
;                                                                      by: ict ;
;==============================================================================;

;>>>>>>>>>>>> ELF header <<<<<<<<<<<<<
EFLheader:
;\
| Member |___________| Type |_| Data |
e_ident	times   0x10    db      0x0
e_type                  dw      0x0
e_machine               dw      0x0
e_version               dd      0x0
e_entry                 dd      0x0
e_phoff                 dd      0x0
e_shoff                 dd      0x0
e_flags                 dd      0x0
e_ehsize                dw      0x0
e_phentsize             dw      0x0
e_phnum                 dw      0x0
e_shentsize             dw      0x0
e_shnum                 dw      0x0
e_shstrndx              dw      0x0

;>>>>>> Program header <<<<<<<
Programheader:
;\
| Member |___| Type |_| Data |
p_type          dd      0x0
p_offset        dd      0x0
p_vaddr         dd      0x0
p_paddr         dd      0x0
p_filesz        dd      0x0
p_memsz         dd      0x0
p_flags         dd      0x0
p_align         dd      0x0
