#include<stdio.h>
#include<stdlib.h>
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
	ret = sendPotato();	
	if(ret)		wait_for_message();
	end_game();
	//joinThreads();
	return 0;
}
