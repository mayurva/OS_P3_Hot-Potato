#include<stdio.h>
#include<stdlib.h>
#include"master.h"

int main(int argc, char *argv[])
{
	int ret;
	if(argc != 4)
	{
		printf("Incorrect set of arguments\n");
		printf("Usage:\n./master <port-number> <number-of-players> <hops>\n");
		exit(-1);
	}
	initMaster(argc,argv);
	setupNetwork();
	wait_for_message();
	printf("\n");
	end_game();
	return 0;
}
