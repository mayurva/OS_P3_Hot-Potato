#include<stdio.h>
#include<stdlib.h>
int main(int argc, char *argv[])
{
        printf("%s\n%s\n%s",argv[1],argv[2],argv[3]);
	if(argc != 4)
	{
		printf("Incorrect set of arguments\n");
		printf("Usage:\n./master <port-number> <number-of-players> <hops>\n");
		exit(-1);
	}
	initMaster(argc,argv);
	setupNetwork();
	sendPotato();	
	joinThreads();
	return 0;
}
