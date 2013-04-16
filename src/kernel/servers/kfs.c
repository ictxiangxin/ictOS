/*================================================================================*/
/*                       ICT Perfect 2.00 kernel file system                      */
/*                                                                        by: ict */
/*================================================================================*/

void init_kfs()
{
	//ict_printf("Setup kernel file system service... ");
	ATADATA ata;
	BYTE buff[512];
	ata.secnum = 0;
	ata.seccnt = 1;
	ata.dev = 0;
	ata.buff = buff;
	send_msg(PID_HDD, HDD_READ, &ata);
	add_kernelproc(kernelfilesystem_daemon, PRIV_KFS);
	//ict_printf("[DONE]\n");
}

void kernelfilesystem_daemon()
{
	MSG m;
	while(TRUE)
	{
		recv_msg(&m);
		switch(m.sig)
		{
		}
	}
}
