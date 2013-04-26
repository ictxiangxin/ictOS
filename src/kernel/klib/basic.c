/*================================================================================*/
/*                           ictOS basic useful functions                         */
/*                                                                        by: ict */
/*================================================================================*/

#include "type.h"
#include "constent.h"

PUBLIC BYTE* ict_strcpy(BYTE* src, BYTE* des)
{
    while((*des++ = *src++) != '\0');
    *des = '\0';
    return des;
}

PUBLIC BYTE* ict_strcpyl(BYTE* src, BYTE* des, DWORD len)
{
    DWORD i;
    for(i = 0; i < len; i++)
        des[i] = src[i];
    return des;
}

PUBLIC DWORD ict_strlen(BYTE* str)
{
    DWORD i = 0x0;
    while(str[i] != '\0')
        i++;
    return i;
}

PUBLIC DWORD ict_strcmp(BYTE* str1, BYTE* str2)
{
    while(*str1 == *str2 && *str1 != '\0')
        str1++, str2++;
    return *str1 == '\0' && *str2 == '\0';
}

PUBLIC DWORD ict_strcmpl(BYTE* str1, BYTE* str2, DWORD len)
{
    DWORD i;
    for(i = 0; i < len; i++)
        if(str1[i] != str2[i])
            return FALSE;
    return TRUE;
}

PUBLIC DWORD ict_ustrlen(WORD* str)
{
    DWORD i = 0x0;
    while(str[i] != '\0')
        i++;
    return i;
}

PUBLIC DWORD ict_ustrcmp(WORD* str1, WORD* str2)
{
    while(*str1 == *str2 && *str1 != '\0')
        str1++, str2++;
    return *str1 == '\0' && *str2 == '\0';
}

PUBLIC DWORD ict_max(DWORD* list, DWORD len)
{
    DWORD max = 0x0;
    DWORD i;
    for(i = 0x1; i < len; i++)
        if(list[max] < list[i])
            max = i;
    return max;
}
