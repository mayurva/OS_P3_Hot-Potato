#include<stdio.h>
#include<pthread.h>

#include"player.h"

int main(int argc,char *argv[])
{
	pthread_t neighbors[2];
	initPlayer(argc,argv);	//this will start the player initializing the data structures and variables
	createConnection(master); //This creates connection with the master
	getNeighbors();
	
	return 0;
}
