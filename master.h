#ifndef MASTER
#define MASTER

#include"potato.h"

extern int num_of_players;
extern player_tracker *player_list;
extern player master;
extern potato p;

char* recvPotato(int,int);

#endif

