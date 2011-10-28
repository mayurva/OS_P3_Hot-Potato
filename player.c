#include<stdio.h>
#include<pthread.h>
#include<stdlib.h>

#include"player.h"

int main(int argc, char *argv[])
{
	if(argc != 3)
	{
		printf("Incorrect set of arguments\n");
		printf("Usage:\n./player <master-machine-name> <port-number>\n");
		exit(-1);
	}

	initPlayer(argv);	//this will start the player initializing the data structures and variables
	
	if(pthread_create(&neighbor_thread[0],NULL,leftNeighborConn,NULL) !=0 )	{
		printf("Thread creation failed\n");
		exit(-1);
	}	
	
	#ifdef DEBUG
		printf("Left Thread created\n");	
	#endif
	
	setupNetwork();

	#ifdef DEBUG
		printPlayerInfo();	
		printf("Out here\n");
	#endif
	
	wait_for_message();

	#ifdef DEBUG
		printf("Exiting program\n");
	#endif

	return 0;
}
