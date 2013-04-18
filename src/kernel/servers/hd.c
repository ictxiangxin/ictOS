/*================================================================================*/
/*                             ictOS hard disk service                            */
/*                                                                        by: ict */
/*================================================================================*/

#include "public.h"
#include "kproc.h"
#include "sig.h"
#include "servers.h"
#include "../io/BIOScolor.h"
#include "../io/ata.h"

#define DEV_PORT(P) (P ^ (pd->device << 0x7))  /* compute the port of the device */

#define _HD_READ    0x1
#define _HD_WRITE   0x2

PRIVATE DWORD lock = FALSE;

PRIVATE VOID wait_hd();
PRIVATE DWORD _ict_handle(DWORD sector_num, DWORD sector_sum, DWORD device, POINTER buff, DWORD handle);
PRIVATE DWORD _hd_read(ATADATA *pd);
PRIVATE DWORD _hd_write(ATADATA *pd);

/******************************************************************/
/* init hard disk service                                         */
/******************************************************************/
PUBLIC VOID init_hd()
{
    //ict_cprintf(COLOR_YELLOW, "Setup hard disk service...      ");
    ict_setupint(0x2e, hd_interrupt);   /* setup the ata interrupt handle */
    //ict_cprintf(COLOR_YELLOW, "[ OK ]\n");
}

/******************************************************************/
/* hard disk daemon                                               */
/******************************************************************/
PUBLIC VOID hd_daemon()
{
    MSG m;
    DWORD err;
    while(TRUE)
    {
        recv_msg(&m);
        switch(m.sig)   /* msg loop */
        {
            case HD_READ :
                err = _hd_read((ATADATA*)m.data);
                if(err)
                    send_msg(m.sproc_id, HD_READERR, err, NULL);
                else
                    send_msg(m.sproc_id, HD_READSUCC, NULL, NULL);
                break;
            case HD_WRITE :
                err = _hd_write((ATADATA*)m.data);
                if(err)
                    send_msg(m.sproc_id, HD_WRITEERR, err, NULL);
                else
                    send_msg(m.sproc_id, HD_WRITESUCC, NULL, NULL);
                break;
        }
        dest_msg ( &m );
    }
}

/******************************************************************/
/* hard disk interrupt                                            */
/******************************************************************/
PUBLIC VOID int_hd()
{
    ict_in(ATA_P_STATUS);
    ict_intfor(PID_HD);
}

/******************************************************************/
/* read hard disk                                                 */
/******************************************************************/
PUBLIC DWORD ict_hdread(DWORD sector_num, DWORD sector_sum, DWORD device, POINTER buff)
{
    return _ict_handle(sector_num, sector_sum, device, buff, _HD_READ);
}

/******************************************************************/
/* write hard disk                                                */
/******************************************************************/
PUBLIC DWORD ict_hdwrite(DWORD sector_num, DWORD sector_sum, DWORD device, POINTER buff)
{
    return _ict_handle(sector_num, sector_sum, device, buff, _HD_WRITE);
}

/******************************************************************/
/* wait hard disk until it isn't busy                             */
/******************************************************************/
PRIVATE VOID wait_hd()
{
    while(ict_in(ATA_P_STATUS) & ATA_STATUS_BSY);   /* just wait the device */
    while(ict_in(ATA_P_STATUS) & ATA_STATUS_DRQ)    /* have data to transfer */
        ict_in(ATA_P_DATA);
    while(ict_in(ATA_P_STATUS) & ATA_STATUS_BSY);   /* just wait the device */
}

/******************************************************************/
/* inferface of hard disk handle                                  */
/******************************************************************/
PRIVATE DWORD _ict_handle(DWORD sector_num, DWORD sector_sum, DWORD device, POINTER buff, DWORD handle)
{
    ATADATA ata;
    ata.secnum = sector_num;
    ata.seccnt = sector_sum;
    ata.device = device;
    ata.buff   = buff;
    if(!ict_lock(&lock)) /* test and lock */
    {
        DWORD err = handle == _HD_READ ? _hd_read(&ata) : _hd_write(&ata);
        ict_unlock(&lock);
        return err;
    }
    MSG m;
    send_msg(PID_HD, handle == _HD_READ ? HD_READ : HD_WRITE, (DWORD)&ata, sizeof(ATADATA));
    recv_msg(&m);
    return m.data;
}

/******************************************************************/
/* read hard disk driver                                          */
/******************************************************************/
PRIVATE DWORD _hd_read(ATADATA *pd)
{
    wait_hd();
    ict_out(DEV_PORT(ATA_P_DEVICECONTROL), 0x0);
    ict_out(DEV_PORT(ATA_P_SECTORCOUNT), pd->seccnt >= 0x100 ? 0x0 : pd->seccnt);
    ict_out(DEV_PORT(ATA_P_LBALOW), pd->secnum & 0xff);
    ict_out(DEV_PORT(ATA_P_LBAMID), pd->secnum >> 0x8 & 0xff);
    ict_out(DEV_PORT(ATA_P_LBAHIGH), pd->secnum >> 0x10 & 0xff);
    ict_out(DEV_PORT(ATA_P_DEVICE), ATA_DEVREG_LBA | ATA_DEVREG_MASTER | (pd->secnum >> 0x18 & 0xf));
    ict_out(DEV_PORT(ATA_P_COMMAND), ATA_CMD_READSECTORS);
    ict_waitint();
    if(ict_in(ATA_P_STATUS) & ATA_STATUS_ERR)
    {
        BYTE err = ict_in(DEV_PORT(ATA_P_ERROR));
        return err;
    }
    ict_ins(DEV_PORT(ATA_P_DATA), pd->buff, pd->seccnt * ATA_SECTORSIZE);
    return NULL;
}

/******************************************************************/
/* write hard disk driver                                         */
/******************************************************************/
PRIVATE DWORD _hd_write(ATADATA *pd)
{
    wait_hd();
    ict_out(DEV_PORT(ATA_P_DEVICECONTROL), 0x0);
    ict_out(DEV_PORT(ATA_P_SECTORCOUNT), pd->seccnt >= 0x100 ? 0x0 : pd->seccnt);
    ict_out(DEV_PORT(ATA_P_LBALOW), pd->secnum & 0xff);
    ict_out(DEV_PORT(ATA_P_LBAMID), pd->secnum >> 0x8 & 0xff);
    ict_out(DEV_PORT(ATA_P_LBAHIGH), pd->secnum >> 0x10 & 0xff);
    ict_out(DEV_PORT(ATA_P_DEVICE), ATA_DEVREG_LBA | ATA_DEVREG_MASTER | (pd->secnum >> 0x18 & 0xf));
    ict_out(DEV_PORT(ATA_P_COMMAND), ATA_CMD_WRITESECTORS);
    ict_outs(DEV_PORT(ATA_P_DATA), pd->buff, pd->seccnt * ATA_SECTORSIZE);
    ict_waitint();
    if(ict_in(ATA_P_STATUS) & ATA_STATUS_ERR)
    {
        BYTE err = ict_in(DEV_PORT(ATA_P_ERROR));
        return err;
    }
    return NULL;
}
