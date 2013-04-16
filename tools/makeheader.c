#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STR_LEN 20

#define LINE   "/******************************************************************************/"
#define AUTHOR "                                               By: ict <ictxiangxin@hotmail.com>"

void usage(char* name)
{
    printf("Usage:\n");
    printf("%s [MappingFile]\n", name);
}

int main(int argc, char **argv)
{
    if(argc != 3)
    {
        usage(argv[0]);
        return 0;
    }
    FILE* mfp = NULL; /* mapping file poniter */
    FILE* ofp = NULL; /* object file pointer */
    char str[MAX_STR_LEN];
    if((mfp = fopen(argv[1], "r")) == NULL)
    {
        fprintf(stderr, "Can not open file !!!\n");
        return -1;
    }
    if((ofp = fopen(argv[2], "r")) == NULL)
    {
        fprintf(stderr, "Can not open file !!!\n");
        return -1;
    }
    fscanf(ofp, "%d", str);
    if(!strcmp(str, LINE))
}
