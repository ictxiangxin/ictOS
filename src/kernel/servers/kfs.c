/*================================================================================*/
/*                       ICT Perfect 2.00 kernel file system                      */
/*                                                                        by: ict */
/*================================================================================*/

void init_kfs()
{
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
