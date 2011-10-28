#include<stdio.h>
#include<pthread.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/select.h>

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
	
	#ifdef DEBUG
		printf("in init master\n");
	#endif
	
	//printf("%s\n%s\n%s",argv[0],argv[1],argv[2]);

	master.listen_port = atoi(argv[1]);

	num_of_players = atoi(argv[2]);
	player_list = (player_tracker*)malloc(num_of_players*sizeof(player_tracker));	
	player_thread = (pthread_t*)malloc(num_of_players*sizeof(pthread_t));
	
	p.hops = atoi(argv[3]);
	p.identities = NULL;
	
	#ifdef DEBUG
		printf("Command line args processed\n");
	#endif

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
		printf("listen error\n");
		exit(-1);
	}

	#ifdef DEBUG
		printf("Master setup\n");
		printf("Master details are\nid: %d\nIP addr: %s\nListen Port: %d\n",master.id,master.ip_addr,master.listen_port);
		printf("Potato is:\n hop count: %d\n",p.hops);
	#endif
}

/*void* listenPlayer(void *args)
{
	printf("Player started\n");
}*/

void print_ring()
{
	int i;
	for(i=0;i<num_of_players;i++)
	{
		#ifdef DEBUG
			printf("Player %d\nIP address: %s\nListen Port: %d\n",player_list[i].id,player_list[i].ip_addr,player_list[i].listen_port);
		#endif
	}
}

void setupNetwork()
{
	struct sockaddr_in sock_client;
	int slen = sizeof(sock_client);
	int i,ret;
	//thread_args args;
	char *a, *b, *c;
	char msg[MAXLEN];
	for(i=0;i<num_of_players;i++)
	{
		player_list[i].id = i;

		if ((player_list[i].conn_port = accept(sock, (struct sockaddr *) &sock_client, &slen)) == -1) {
			printf("accept call failed! \n");
			exit(-1);
		}
	
		ret = recv(player_list[i].conn_port, msg, MAXLEN, 0);
		if ( ret == -1 )
		{
			printf("Receive error! \n");
			exit(-1);
		}
		else if ( ret == 0)
		{
			close(player_list[i].conn_port);
			continue;
		}
		
		a = strtok(msg,"\n");

		#ifdef DEBUG
			printf("Received %s message from player %d\n",a,i);
		#endif

		if(strcmp(a,"JOIN")==0)
		{
			a = strtok(NULL,"\n");
			strcpy(player_list[i].ip_addr,a);
			a = strtok(NULL,"\n");
			player_list[i].listen_port = atoi(a);
		}
	
//		pthread_create(&(player_thread[i]),NULL,listenPlayer,(void *)&args);

	}

	#ifdef DEBUG	
		printf("All players joined\n");
	#endif

	close(sock);
	for(i=0;i<num_of_players;i++)
	{
		sprintf(msg,"INFO\n%d\n%d\n%s\n%d\n",i,(i+1)%num_of_players,player_list[(i+1)%num_of_players].ip_addr,player_list[(i+1)%num_of_players].listen_port);
		if(send(player_list[i].conn_port,msg,strlen(msg),0)==-1)
		{
			printf("Sending failed\n");
			exit(-1);
		}
		#ifdef DEBUG
			printf("INFO message sent to player %d\n",i);	
		#endif
	}

	#ifdef DEBUG
		printf("The ring is\n");
		print_ring();
	#endif	

//	sendPotato();
}

void wait_for_message()
{
	int i;
	char msg[MAXLEN];
	char *a,*b;
	fd_set readset;

	while(1)
	{
		for(i=0;i<num_of_players;i++)
	                FD_SET(player_list[i].conn_port,&readset);
                select(0,&readset,NULL,NULL,NULL);
		
                for(i=0;i<num_of_players;i++)
			if(FD_ISSET(player_list[i].conn_port,&readset))
				break;

		if(recv(player_list[i].conn_port, msg, MAXLEN, 0) == -1)
		{
			printf("Receive error! \n");
			exit(-1);
		}
		a = strtok_r(msg,"\n",&b);

		#ifdef DEBUG
			printf("Received %s message from player %d\n",a,i);
		#endif		

		if(strcmp(a,"POTA")==0)
		{
			int n;
			p.identities = malloc(MAXLEN*sizeof(char));
			a = strtok_r(NULL,"\n",&b);
			p.hops = atoi(a);
			strcpy(p.identities,b);

			#ifdef DEBUG
				printf("\nReceived Potato\nPrinting trace now\n");
			#endif
			a = strtok(p.identities,"\n");
			while(a)
			{
				n=atoi(a);
				printf("%d\n",n);
				a = strtok(NULL,"\n");
			}
			printf("\n");		
			break;
		}
	}
	
}

void end_game()
{
	int i;
	char msg[MAXLEN];
	sprintf(msg,"TERM");

	for(i=0;i<num_of_players;i++)
	{
		if(send(player_list[i].conn_port,msg,strlen(msg),0)==-1)
                {
			printf("Sending failed\n");
			exit(-1);
		}
		#ifdef DEBUG
			printf("Terminate message sent to player %d\n",i);
		#endif
	}	

	for(i=0;i<num_of_players;i++)
		close(player_list[i].conn_port);
	#ifdef DEBUG
		printf("Exiting the master\n");
	#endif
}

int sendPotato()
{
	char msg[MAXLEN];
	if(p.hops)
	{
		int player_id = rand()%num_of_players;

		#ifdef DEBUG
			printf("Sending Potato to player %d\n",player_id);
		#endif	

		sprintf(msg,"POTA\n%d\n",p.hops);
		if(send(player_list[player_id].conn_port,msg,strlen(msg),0)==-1)
		{
			printf("Sending failed\n");
			exit(-1);
		}
		#ifdef DEBUG
			printf("Sent Potato:\n%s\n",msg);
		#endif
	}
	return p.hops;
}

void joinThreads()
{
	int i;
	for(i=0;i<num_of_players;i++)
		pthread_join(player_thread[i],NULL);
}
