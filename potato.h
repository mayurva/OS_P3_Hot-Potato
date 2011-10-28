#ifndef POTATO
#define POTATO

//#define DEBUG
#define MAXLEN 1000
#define TRUE 1
#define FALSE 0

typedef struct player_tracker_{
	int id;
	char ip_addr[128];
	int conn_port;
	int listen_port;
} player_tracker;

typedef struct player_{
	int id;	
	char ip_addr[128];
	char iface_name[64];
	int listen_port;
} player;

typedef struct potato_{
	int hops;
	char *identities;
} potato;
typedef struct thread_args_{
}thread_args;

player populatePublicIp(player);
char* receiveMessage(player_tracker);


/*typedef struct message_{
} message;*/

#endif
