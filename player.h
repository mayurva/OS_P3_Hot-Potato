#ifndef PLAYER
#define PLAYER

#include<pthread.h>

#include"potato.h"

extern player_tracker master;
extern player_tracker neighbor[2];
extern player me;
extern potato p;

extern pthread_t neighbor_thread[2];
extern pthread_t master_thread;

void* masterConn(void*);
void* leftNeighborConn(void*);

#endif
