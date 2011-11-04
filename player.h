#ifndef PLAYER
#define PLAYER

#include<pthread.h>

#include"potato.h"

extern player_tracker master;
extern player_tracker neighbor[2];
extern player me;
extern potato p;
extern pthread_t left_thread;

void* leftNeighborConn(void*);
char* recvPotato(int,int);

#endif
