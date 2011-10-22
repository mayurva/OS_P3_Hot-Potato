#ifndef POTATO
#define POTATO

typedef struct player_tracker_{
	int id;
	char ip_addr[128];
	int portNum;
} player_tracker;

typedef struct player_{
	int id;	
	char ip_addr[128];
	char iface_name[64];
	int portNum;
} player;

typedef struct potato_{
	int hops;
	char *identites;
} potato;

/*typedef struct message_{
} message;*/

#endif
