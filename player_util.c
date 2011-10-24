#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include<pthread.h>

#include"potato.h"
#include"player.h"

player_tracker master;
player me;
player_tracker neighbor[2];
int sock;
int termflag;

pthread_t neighbor_thread[2];
pthread_t master_thread;

int initPlayer(char *argv[])
{
	master.id = 0;
	strcpy(master.ip_addr,argv[1]);
	printf("%s\n",argv[2]);
	master.listen_port = atoi(argv[2]);

	#ifdef DEBUG
		printf("Master port num is %d\n",master.listen_port);
		printf("Command line args processed\n");
	#endif
	
	me = populatePublicIp(me);
	
	if ((me.listen_port = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		printf("error in socket creation");
		exit(-1);
	}
	
	#ifdef DEBUG
		printf("Listen Socket created\n");
	#endif

        if ((master.conn_port = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
                printf("error in socket creation");
                exit(-1);
        }
	termflag = 0;

	#ifdef DEBUG
		printf("Connection Socket created\n");
		printf("Player initialized\n");
	#endif
}

void setNeighborData(int i,char *msg)
{
}

void processPotato()
{
	/*while(!termflag)
	{
		//receive
		while(1)
		{
			//receive potato
			//buffer potato
		}	
		//attach identity
		//send potato
	}*/
}

void* rightNeighborConn(void *args)
{
	createConnection(neighbor[1]);
	processPotato();
}

void* leftNeighborConn(void *args)
{
	struct sockaddr_in sock_client;
	int slen = sizeof(sock_client);
	char msg[MAXLEN];
	
	#ifdef DEBUG
		printf("Listen for connection\n");
	#endif

        if(listen(me.listen_port, 1) == -1) {
                printf("listen error");
                exit(-1);
        }

	if ((neighbor[0].conn_port = accept(me.listen_port, (struct sockaddr *) &sock_client, &slen)) == -1) {
		printf("accept call failed! \n");
		exit(-1);
	}
	
	#ifdef DEBUG
		printf("Connection accepted\n");
	#endif
	
	strcpy(msg,receiveMessage(neighbor[0]));
	setNeighborData(0,msg);
	processPotato();
}

void* masterConn(void *args)
{
	char msg[MAXLEN];
	createConnection(master);

	#ifdef DEBUG
		printf("Connected to master\n");
	#endif

	sendMessage(master,"JOIN");

	#ifdef DEBUG
		printf("JOIN message sent\n");
	#endif

	strcpy(msg,receiveMessage(master));
	/*setNeighborData(1,msg);	
	if(pthread_create(&neighbor_thread[1],NULL,rightNeighborConn,NULL) !=0 ) {     
                printf("Thread creation failed\n");

                exit(-1);
        }
	strcpy(msg,receiveMessage(master));
	termflag = 1;
        if(pthread_join(neighbor_thread[1],NULL) !=0 ) {
                printf("Join failed\n");
                exit(-1);
        }*/

}
