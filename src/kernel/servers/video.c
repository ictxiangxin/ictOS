/*================================================================================*/
/*                         ICT Perfect 2.00 video service                         */
/*                                                                        by: ict */
/*================================================================================*/

#include "kasm.h"
#include "public.h"
#include "constent.h"
#include "type.h"
#include "sig.h"
#include "kproc.h"
#include "../io/BIOScolor.h"

#define _VD_CALL 0x1
#define _VD_MSG  0X2

PRIVATE VOID _refresh();
PRIVATE VOID _putchar ( DWORD data );
PRIVATE VOID _putstring ( DWORD data, DWORD datasize );
PRIVATE VOID _dropchar ( DWORD data );
PRIVATE VOID _ict_dropchar(BYTE c, BYTE color, DWORD x, DWORD y, DWORD handle);

PRIVATE VOID call_cprintf ( BYTE color, POINTER pformat, BYTE *format, ... );
PRIVATE VOID _ict_call_printdn ( LINT n, DWORD color );
PRIVATE VOID _ict_call_printhon ( DWORD n, DWORD base, DWORD color );
PRIVATE VOID msg_cprintf ( BYTE color, POINTER pformat, BYTE* format, ... );
PRIVATE VOID _ict_msg_printdn ( LINT n, BYTE* color, DWORD* bp ); /* print a number and this function only use by ict_printf() */
PRIVATE VOID _ict_msg_printhon ( DWORD n, DWORD base, BYTE* color, DWORD* bp );   /* print unsigned hex or octal number */

PRIVATE DWORD cursorlocation;
PRIVATE DWORD startlocation;
PRIVATE DWORD video_width;
PRIVATE DWORD video_high;

PRIVATE DWORD lock = FALSE; /* video lock */

/******************************************************************/
/* init video service                                             */
/******************************************************************/
PUBLIC VOID init_video()
{
    cursorlocation = ict_cursorlocation();
    startlocation = ict_startlocation();
    video_width = 0x50;
    video_high = 0x19;
    ict_refreshvideo ( startlocation, video_width, video_high, GRAPHIC_BUFF_SIZE );
}

/******************************************************************/
/* video daemon                                                   */
/******************************************************************/
PUBLIC VOID video_daemon()
{
    MSG m;
    while ( TRUE ) /* msg loop */
    {
        recv_msg ( &m ); /* wait a msg */
        while(ict_lock(&lock))
            ict_done();
        switch ( m.sig )
        {
        case VD_CHAR :
            _putchar ( m.data );
            break;
        case VD_STRING :
            _putstring ( m.data, m.datasize );
            break;
        case VD_DROPCHAR :
            _dropchar ( m.data );
            break;
        case VD_TEXTMODE :
            break;
        case VD_GRAPHMODE :
            break;
        }
        dest_msg ( &m );
        ict_unlock(&lock);
    }
}

/******************************************************************/
/* print a char on screen                                         */
/******************************************************************/
PUBLIC VOID ict_putchar(BYTE c)
{
    DWORD tmp = DEFAULT_COLOR;
    tmp <<= 0x8;
    tmp += c;
    _putchar(tmp);
}

/******************************************************************/
/* print a char on screen with color                              */
/******************************************************************/
PUBLIC VOID ict_cputchar(BYTE c, BYTE color)
{
    DWORD tmp = color;
    tmp <<= 0x8;
    tmp += c;
    if(!ict_lock(&lock))
    {
        _putchar(tmp);
        ict_unlock(&lock);
    }
    send_msg(PID_VD, VD_CHAR, tmp, NULL);
}

/******************************************************************/
/* printf function                                                */
/******************************************************************/
PUBLIC VOID ict_printf(BYTE* format, ...)
{
    if(!ict_lock(&lock)) /* test and lock */
    {
        call_cprintf(DEFAULT_COLOR, &format, format);
        ict_unlock(&lock);
        return;
    }
    msg_cprintf(DEFAULT_COLOR, &format, format);
}

/******************************************************************/
/* printf function with color                                     */
/******************************************************************/
PUBLIC VOID ict_cprintf(BYTE color, BYTE* format, ...)
{
    if(!ict_lock(&lock)) /* test and lock */
    {
        call_cprintf(color, &format, format);
        ict_unlock(&lock);
        return;
    }
    msg_cprintf(color, &format, format);
}

/******************************************************************/
/* drop char by directly call                                     */
/******************************************************************/
PUBLIC VOID call_dropchar(BYTE c, BYTE color, DWORD x, DWORD y)
{
    _ict_dropchar(c, color, x, y, _VD_CALL);
}

/******************************************************************/
/* drop char by sending msg                                       */
/******************************************************************/
PUBLIC VOID msg_dropchar(BYTE c, BYTE color, DWORD x, DWORD y)
{
    _ict_dropchar(c, color, x, y, _VD_MSG);
}

/******************************************************************/
/* refresh video arguments                                        */
/******************************************************************/
PRIVATE VOID _refresh()
{
    while ( cursorlocation >= startlocation + video_width * video_high )
        startlocation += video_width;
    if ( startlocation + video_width * video_high >= GRAPHIC_BUFF_SIZE - video_width * video_high )
    {
        ict_refreshvideo ( startlocation, video_width, video_high, GRAPHIC_BUFF_SIZE );
		cursorlocation -= startlocation;
        startlocation = 0;
    }
    ict_setcursor ( cursorlocation );
    ict_setstart ( startlocation );
}

/******************************************************************/
/* putchar function used by this service                          */
/******************************************************************/
PRIVATE VOID _putchar ( DWORD data )
{
    if ( ( data & 0xff ) == '\n' )
        cursorlocation = ( cursorlocation / video_width + 1 ) * video_width;
    else
    {
        ict_putc ( data, cursorlocation );
        cursorlocation++;
    }
    _refresh();
}

/******************************************************************/
/* putstring function used by this service                        */
/******************************************************************/
PRIVATE VOID _putstring ( DWORD data, DWORD datasize )
{
    DWORD i = 0x0;
    WORD color = ( WORD ) ( ( ( BYTE* ) data ) [0] << 0x8 );
    for ( i = 0x1; i < datasize; i++ )
        ict_putc ( color + ( WORD ) ( ( BYTE* ) data ) [i], cursorlocation + i - 0x1 );
    cursorlocation += datasize - 0x1;
    _refresh();
}

/******************************************************************/
/* dropchar function used by this service                         */
/******************************************************************/
PRIVATE VOID _dropchar ( DWORD data )
{
    DWORD x = ( DWORD ) ( ( BYTE* ) ( &data ) ) [2];
    DWORD y = ( DWORD ) ( ( BYTE* ) ( &data ) ) [3];
    DWORD c = ( DWORD ) ( ( WORD* ) ( &data ) ) [0];
    ict_putc ( c, startlocation + x * video_width + y );
}

/******************************************************************/
/* drop a char on screen with two ways                            */
/******************************************************************/
PRIVATE VOID _ict_dropchar(BYTE c, BYTE color, DWORD x, DWORD y, DWORD handle)
{
    x %= video_high;
    y %= video_width;
    DWORD data;
    ((BYTE*)(&data))[0] = c;
    ((BYTE*)(&data))[1] = color;
    ((BYTE*)(&data))[2] = x;
    ((BYTE*)(&data))[3] = y;
    if(handle == _VD_CALL)
        _dropchar(data);
    else
        send_msg ( PID_VD, VD_DROPCHAR, data, NULL );
}

/******************************************************************/
/* works as the printf in c with color                            */
/******************************************************************/
PRIVATE VOID call_cprintf ( BYTE color, POINTER pformat, BYTE *format, ... )
{
    LINT a = ( LINT ) ( pformat );	/* create the pointer to the first argument */
    a += sizeof(POINTER);	/* let a point to the next argument */
    DWORD tmp_color = color;
    tmp_color <<= 0x8;
    DWORD i = 0;	/* create i for next loop */
    for ( i = 0; format[i] != '\0'; i++ )	/* until the string end('\0') */
    {
        if ( format[i] == '%' )	/* special elem in the string */
        {
            i++;	/* get the type of the elem */
            if ( format[i] == 's' )	/* it is string */
            {
                DWORD _i = 0;	/* for next loop */
                /* translate the a to the BYTE**, it can point to
                   the BYTE*,and use '*' to get the string(BYTE *),
                   so we get the string we will print */
                BYTE *str = * ( BYTE** ) a;
                for ( _i = 0; str[_i] != '\0'; _i++ )	/* print all string */
                    _putchar ( str[_i] + tmp_color );	/* use the basic function print one BYTE */
                a += sizeof(POINTER);	/* let a point to the next argument */
            }
            if ( format[i] == 'c' )	/* it is a BYTE */
            {
                _putchar ( * ( BYTE* ) a + tmp_color );	/* translate a to BYTE* and get the BYTE */
                a += sizeof(POINTER);	/* let a point to the next argument */
            }
            if ( format[i] == 'x' || format[i] == 'o' )	/* it is a unsigned number */
            {
                switch ( format[i] )	/* choose the number system */
                {
                case 'x' :	/* it is hex number */
                    _ict_call_printhon ( * ( DWORD* ) a, 16, tmp_color );
                    break;
                case 'o' :	/* it is octal number */
                    _ict_call_printhon ( * ( DWORD* ) a, 8, tmp_color );
                    break;
                }
                a += sizeof(POINTER);	/* let a point to the next argument */
            }
            if ( format[i] == 'd' )	/* it is decimal number */
            {
                _ict_call_printdn ( * ( LINT* ) a, tmp_color );	/* use special function to print it */
                a += sizeof(POINTER);	/* let a point to the next argument */
            }
            continue;	/* to handle next BYTE */
        }
        _putchar ( format[i] + tmp_color );	/* it is a normal BYTE, and just print it */
    }
}

/******************************************************************/
/* print the signed decimal number                                */
/******************************************************************/
PRIVATE VOID _ict_call_printdn ( LINT n, DWORD color )	/* print signed decimal number */
{
    if ( n & 0x80000000 )	/* if it is a negetive number */
    {
        n ^= 0xffffffff;	/* translate to positive number */
        n++;			/*                              */
        _putchar ( '-' + color );	/* print the negetive sym */
    }
    DWORD a = 0;	/* save the higher number */
    if ( ( a = n / 10 ) != 0 )	/* compute the higher number and if it is 0 */
        _ict_call_printdn ( a, color );	/* it is not 0 and we need call _ict_call_printdn() to print it */
    _putchar ( '0' + ( n % 10 ) + color );	/* it is not hex number or it is not a BYTE, print it normally */
}

/******************************************************************/
/* print the hex or octal number                                  */
/******************************************************************/
PRIVATE VOID _ict_call_printhon ( DWORD n, DWORD base, DWORD color )
{
    DWORD a = 0;	/* save the higher number */
    if ( ( a = n / base ) != 0 )	/* compute the higher number and if it is 0 */
        _ict_call_printhon ( a, base, color );	/* it is not 0 and we need call _ict_call_printhon() to print it */
    if ( ( n % base ) > 9 )	/* if it is a hex number and this byte is a BYTE */
        _putchar ( 'a' + ( n % base ) - 10 + color );	/* translate it to a hex BYTE and print it */
    else
        _putchar ( '0' + ( n % base ) + color );	/* it is not a BYTE, print it normally */
}

PRIVATE VOID msg_cprintf ( BYTE color, POINTER pformat, BYTE* format, ... )
{
    BYTE tmpbuff[PRINT_BUFF] = {0x0};
    DWORD bp = 0x1;
    tmpbuff[0x0] = color;
    DWORD ptr = ( DWORD ) ( pformat );    /* create the pointer to the first argument */
    ptr += sizeof ( POINTER ); /* let a point to the next argument */
    DWORD i = 0;  /* create i for next loop */
    for ( i = 0; format[i] != '\0'; i++ )   /* until the string end('\0') */
    {
        if ( format[i] == '%' ) /* special elem in the string */
        {
            i++;    /* get the type of the elem */
            if ( format[i] == 's' ) /* it is string */
            {
                DWORD _i = 0; /* for next loop */
                /* translate the a to the char**, it can point to
                   the char*,and use '*' to get the string(char *),
                   so we get the string we will print */
                BYTE *str = * ( BYTE** ) ptr;
                for ( _i = 0; str[_i] != '\0'; _i++ )   /* print all string */
                    tmpbuff[bp++] = str[_i]; /* use the basic function print one char */
                ptr += sizeof ( POINTER ); /* let a point to the next argument */
            }
            if ( format[i] == 'c' ) /* it is a char */
            {
                tmpbuff[bp++] = * ( BYTE* ) ptr;   /* translate a to char* and get the char */
                ptr += sizeof ( POINTER ); /* let a point to the next argument */
            }
            if ( format[i] == 'x' || format[i] == 'o' ) /* it is a unsigned number */
            {
                switch ( format[i] )    /* choose the number system */
                {
                case 'x' :  /* it is hex number */
                    _ict_msg_printhon ( * ( DWORD* ) ptr, 16, tmpbuff, &bp );
                    break;
                case 'o' :  /* it is octal number */
                    _ict_msg_printhon ( * ( DWORD* ) ptr, 8, tmpbuff, &bp );
                    break;
                }
                ptr += sizeof ( POINTER ); /* let a point to the next argument */
            }
            if ( format[i] == 'd' ) /* it is decimal number */
            {
                _ict_msg_printdn ( * ( LINT* ) ptr, tmpbuff, &bp );    /* use special function to print it */
                ptr += sizeof ( POINTER ); /* let a point to the next argument */
            }
            continue;   /* to handle next char */
        }
        if ( format[i] == '\n' )
        {
            send_msg ( PID_VD, VD_STRING, tmpbuff, bp );
            bp = 0x1;
            send_msg ( PID_VD, VD_CHAR, format[i], NULL );
        }
        else
            tmpbuff[bp++] = format[i];   /* it is a normal char, and just print it */
    }
    send_msg ( PID_VD, VD_STRING, tmpbuff, bp );
}

/******************************************************************/
/* print the signed decimal number                                */
/******************************************************************/
PRIVATE VOID _ict_msg_printdn ( LINT n, BYTE* tmpbuff, DWORD* bp )  /* print signed decimal number */
{
    if ( n & 0x80000000 )   /* if it is a negetive number */
    {
        n ^= 0xffffffff;    /* translate to positive number */
        n++;            /*                              */
        tmpbuff[ ( *bp ) ++] = '-'; /* print the negetive sym */
    }
    DWORD a = 0;  /* save the higher number */
    if ( ( a = n / 10 ) != 0 )  /* compute the higher number and if it is 0 */
        _ict_msg_printdn ( a, tmpbuff, bp );  /* it is not 0 and we need call _ict_msg_printdn() to print it */
    tmpbuff[ ( *bp ) ++] = '0' + ( n % 10 ); /* it is not hex number or it is not a char, print it normally */
}

/******************************************************************/
/* print the hex or octal number                                  */
/******************************************************************/
PRIVATE VOID _ict_msg_printhon ( DWORD n, DWORD base, BYTE* tmpbuff, DWORD* bp )
{
    DWORD a = 0; /* save the higher number */
    if ( ( a = n / base ) != 0 )    /* compute the higher number and if it is 0 */
        _ict_msg_printhon ( a, base, tmpbuff, bp );   /* it is not 0 and we need call _ict_msg_printhon() to print it */
    if ( ( n % base ) > 9 ) /* if it is a hex number and this byte is a char */
        tmpbuff[ ( *bp ) ++] = 'a' + ( n % base ) - 10; /* translate it to a hex char and print it */
    else
        tmpbuff[ ( *bp ) ++] = '0' + ( n % base ); /* it is not a char, print it normally */
}
