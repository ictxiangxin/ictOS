/*================================================================================*/
/*                       ICT Perfect 2.00 hard disk works                         */
/*                                                                        by: ict */
/*================================================================================*/

#include	"../io/ata.h"
#include	"public.h"
#include	"kproc.h"
#include	"sig.h"
#include	"servers.h"
#include	"../io/BIOScolor.h"

#define DEV_PORT(P)	(P ^ (pd->dev << 0x7))	/* compute the port of the device */

PUBLIC VOID hd_interrupt();
PUBLIC VOID hd_daemon();

PUBLIC VOID init_hd()
{
	//ict_cprintf(COLOR_YELLOW, "Setup hard disk service...      ");
	ict_setupint(0x2e, hd_interrupt);	/* setup the ata interrupt handle */
	//ict_cprintf(COLOR_YELLOW, "[ OK ]\n");
}

PUBLIC VOID int_hd()
{
	ict_in(ATA_P_STATUS);
	ict_intfor(PID_HDD);
}

PUBLIC VOID wait_harddisk()
{
	while(ict_in(ATA_P_STATUS) & ATA_STATUS_BSY);	/* just wait the device */
	while(ict_in(ATA_P_STATUS) & ATA_STATUS_DRQ)	/* have data to transfer */
		ict_in(ATA_P_DATA);
	while(ict_in(ATA_P_STATUS) & ATA_STATUS_BSY);	/* just wait the device */
}

void hd_test()
{
	wait_harddisk();
	ict_out(ATA_P_DEVICECONTROL, 0x0);
	ict_out(ATA_P_DEVICE, 0x00);
	ict_out(ATA_P_COMMAND, ATA_CMD_IDENTIFY);
	ict_waitint();
	short int buff[512];
	ict_ins(ATA_P_DATA, buff, 512);
	ict_cprintf(COLOR_LIGHTRED, "\n======== Hard Disk Information ========\n");
	if(buff[0] & 0x40)
		ict_cprintf(COLOR_LIGHTRED, "Not Removable Controller and/or Device\n");
	else if(buff[0] & 0x80)
		ict_cprintf(COLOR_LIGHTRED, "Removable Media Device\n");
	if(!(buff[0] & 0x8000))
		ict_cprintf(COLOR_LIGHTRED, "<ATA Device> ");
	ict_cprintf(COLOR_LIGHTRED, "C:%d H:%d S:%d\n", buff[1], buff[3], buff[6]);
	ict_cprintf(COLOR_LIGHTRED, "Supports:");
	if(buff[80] & 0x1)
		ict_cprintf(COLOR_LIGHTRED, " ATA-1 ");
	if(buff[80] & 0x2)
		ict_cprintf(COLOR_LIGHTRED, " ATA-2 ");
	if(buff[80] & 0x3)
		ict_cprintf(COLOR_LIGHTRED, " ATA-3 ");
	if(buff[80] & 0x4)
		ict_cprintf(COLOR_LIGHTRED, " ATA/ATAPI-4 ");
	ict_cprintf(COLOR_LIGHTRED, "\nSerial Number:");
	int i;
	for(i = 0; i < 10; i++)
	{
		ict_cputchar(buff[10 + i] & 0xff, COLOR_LIGHTRED);
		ict_cputchar(buff[10 + i] >> 0x8, COLOR_LIGHTRED);
	}
	ict_cprintf(COLOR_LIGHTRED, "\nFirmware Revision:");
	for(i = 0; i < 4; i++)
	{
		ict_cputchar(buff[23 + i] & 0xff, COLOR_LIGHTRED);
		ict_cputchar(buff[23 + i] >> 0x8, COLOR_LIGHTRED);
	}
	ict_cprintf(COLOR_LIGHTRED, "\nModel Number:");
	for(i = 0; i < 20; i++)
	{
		ict_cputchar(buff[27 + i] & 0xff, COLOR_LIGHTRED);
		ict_cputchar(buff[27 + i] >> 0x8, COLOR_LIGHTRED);
	}
	ict_cprintf(COLOR_LIGHTRED, "\n");
}

DWORD hd_read(ATADATA *pd)
{
	wait_harddisk();
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
	return 0;
}

DWORD hd_write(ATADATA *pd)
{
	wait_harddisk();
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
	return 0;
}

PUBLIC VOID hd_daemon()
{
	MSG m;
	DWORD err;
	while(TRUE)
	{
		recv_msg(&m);
		switch(m.sig)	/* msg loop */
		{
			case 0 :
				hd_test();
				break;
			case HDD_READ :
				err = hd_read((ATADATA*)m.data);
				if(err)
					send_msg(m.sproc_id, HDD_READERR, err, NULL);
				else
					send_msg(m.sproc_id, HDD_READSUCC, NULL, NULL);
				break;
			case HDD_WRITE :
				err = hd_write((ATADATA*)m.data);
				if(err)
					send_msg(m.sproc_id, HDD_WRITEERR, err, NULL);
				else
					send_msg(m.sproc_id, HDD_WRITESUCC, NULL, NULL);
				break;
		}
	}
}
