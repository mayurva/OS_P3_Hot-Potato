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
	
	if(pthread_create(&master_thread,NULL,masterConn,NULL) !=0 ) {
		printf("Thread creation failed\n");
		exit(-1);
	}	
	
	#ifdef DEBUG
		printf("Master Thread created\n");	
	#endif

	if(pthread_join(neighbor_thread[0],NULL) !=0 ) {
		printf("Join failed\n");
		exit(-1);
	}
	
	if(pthread_join(master_thread,NULL) !=0 ) {
		printf("Join failed\n");
		exit(-1);
	}
	
	#ifdef DEBUG
		printf("Exiting program\n");
	#endif

	return 0;
}
