/*================================================================================*/
/*                 ICT Perfect 2.00 kernel mode string functions                  */
/*                                                                        by: ict */
/*================================================================================*/

char* ict_strcpy(char* scr, char* des)
{
	char* tmp_s = scr;
	char* tmp_d = des;
	while((*tmp_d++ = *tmp_s++) != '\0');
	*tmp_d = '\0';
	return des;
}

char* ict_strcpyl(char* scr, char* des, int len)
{
	int i;
	for(i = 0; i < len; i++)
		des[i] = scr[i];
	return des;
}
