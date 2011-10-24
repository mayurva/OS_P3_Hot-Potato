#include<stdio.h>
#include<pthread.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include"potato.h"
#include"master.h"


potato p;
int num_of_players;
player master;
player_tracker *player_list;
pthread_t *player_thread;
int sock;

void initMaster(int argc,char *argv[])
{
	struct sockaddr_in sock_server;

	master.id = 0;
	master = populatePublicIp(master);
	
	printf("in init master\n");
	
	printf("%s\n%s\n%s",argv[0],argv[1],argv[2]);

	master.listen_port = atoi(argv[1]);

	num_of_players = atoi(argv[2]);
	player_list = (player_tracker*)malloc(num_of_players*sizeof(player_tracker));	
	player_thread = (pthread_t*)malloc(num_of_players*sizeof(pthread_t));
	
	p.hops = atoi(argv[3]);
	p.identities = NULL;
	
	printf("Command line args processed\n");

	if((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		printf("error in socket creation\n");
		exit(-1);
	}

	memset((char *) &sock_server, 0, sizeof(sock_server));
	sock_server.sin_family = AF_INET;
	sock_server.sin_port = htons(master.listen_port);
	sock_server.sin_addr.s_addr = inet_addr(master.ip_addr);

	if (bind(sock, (struct sockaddr *) &sock_server, sizeof(sock_server)) == -1) {
		printf("Bind failed\n");
		exit(-1);
	}

	if(listen(sock, num_of_players) == -1) {
		printf("listen error");
		exit(-1);
	}

	#ifdef DEBUG
		printf("Master setup\n");
		printf("Master details are\nid: %d\nIP addr: %s\nListen Port: %d\n",master.id,master.ip_addr,master.listen_port);
		printf("Potato is:\n hop count: %d\n",p.hops);
	#endif
}

void* listenPlayer(void *args)
{
	
	printf("Player started\n");
}

void setupNetwork()
{
	struct sockaddr_in sock_client;
	int client;
	int slen = sizeof(sock_client);
	int i,ret;
	thread_args args;
	char msg[MAXLEN];
	for(i=0;i<num_of_players;i++)
	{
		if ((client = accept(sock, (struct sockaddr *) &sock_client, &slen)) == -1) {
			printf("accept call failed! \n");
			exit(-1);
		}
	
		ret = recv(client, msg, MAXLEN, 0);

		if ( ret == -1 )
		{
			printf("Receive error! \n");
			exit(-1);
		}
		else if ( ret == 0)
		{
			close(client);
			continue;
		}

		//save join msg coming from node	
		
		pthread_create(&(player_thread[i]),NULL,listenPlayer,(void *)&args);

	}
	printf("All players joined\n");
}

void sendPotato()
{
}

void joinThreads()
{
	int i;
	for(i=0;i<num_of_players;i++)
		pthread_join(player_thread[i],NULL);
}
