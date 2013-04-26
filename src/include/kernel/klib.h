/*================================================================================*/
/*                               ictOS Kernel Lib                                 */
/*                                                                        by: ict */
/*================================================================================*/

/* protect */
PUBLIC VOID ict_Descinit(SEGDESC* sd, DWORD base, DWORD limit, WORD attr);

/* basic */
PUBLIC BYTE* ict_strcpy(BYTE* src, BYTE* des);
PUBLIC BYTE* ict_strcpyl(BYTE* src, BYTE* des, DWORD len);
PUBLIC DWORD ict_strlen(BYTE* str);
PUBLIC DWORD ict_strcmp(BYTE* str1, BYTE* str2);
PUBLIC DWORD ict_strcmpl(BYTE* str1, BYTE* str2, DWORD len);
PUBLIC DWORD ict_ustrlen(WORD* str);
PUBLIC DWORD ict_ustrcmp(WORD* str1, WORD* str2);
PUBLIC DWORD ict_max(DWORD* list, DWORD len);
