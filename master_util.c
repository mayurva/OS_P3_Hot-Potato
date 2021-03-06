#include<stdio.h>
#include<pthread.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<sys/select.h>

#include"potato.h"
#include"master.h"

potato p;
int num_of_players;
player master;
player_tracker *player_list;
int *network_setup;
int sock;
fd_set readset;

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
			printf("Terminate message sent to player %d\n",i+1);
		#endif
	}	

	for(i=0;i<num_of_players;i++)
	{
		shutdown(player_list[i].conn_port,SHUT_RDWR);
		close(player_list[i].conn_port);
	}
}

int handleMessage(int i,char msg[])
{
	char *a,*b;
	int n;
	
	if(msg)
	{
		a = strtok_r(msg,"\n",&b);
		#ifdef DEBUG
			printf("Received %s message from player %d\n",a,i+1);
		#endif		

		if(strcmp(a,"POTA")==0)
		{
			a = strtok_r(NULL,"\n",&b);
			p.hops = atoi(a);
			a = strtok_r(NULL,"\n",&b);

			p.identities=(char*)recvPotato(player_list[i].conn_port,atoi(a));
			end_game();
			printf("Trace of Potato:\n");
			a = strtok(p.identities,"\n");
			while(a)
			{
				n=atoi(a);
					printf("%d",n);
				a = strtok(NULL,"\n");
				if(a==NULL)
					break;
				printf(",");
			}
			free(p.identities);
			return 0;
		}
		else if(strcmp(a,"CONN")==0)
		{
			a = strtok_r(NULL,"\n",&b);
			n = atoi(a);
		
			#ifdef DEBUG
				printf("n is %d\n",n);
			#endif
	
			network_setup[n-1] = 1;
			if(networkSetup())
			{
				if(p.hops)
				{
					int player_id = getRandom(1,num_of_players+1);
					printf(", Sending Potato to player %d\n",player_id);
					sprintf(msg,"POTA\n%d\n%d\n",p.hops,0);
					sendPotato(p,msg,player_list[player_id-1].conn_port);
				}
				else
					return 0;
			}
		}
		else if(strcmp(a,"JOIN")==0)
		{
			a = strtok_r(NULL,"\n",&b);
			strcpy(player_list[i].ip_addr,a);
			a = strtok_r(NULL,"\n",&b);
			player_list[i].listen_port = atoi(a);
		}
	}
	return 1;
}

void initMaster(int argc,char *argv[])
{
	int i;
	master.id = 0;
	master = populatePublicIp(master);
	
	#ifdef DEBUG
		printf("in init master\n");
	#endif
	
	master.listen_port = atoi(argv[1]);

	num_of_players = atoi(argv[2]);
	player_list = (player_tracker*)malloc(num_of_players*sizeof(player_tracker));	
	network_setup = malloc(num_of_players*sizeof(int));
	for(i=0;i<num_of_players;i++)
		network_setup[i] = 0;
	
	p.hops = atoi(argv[3]);
	p.identities = NULL;
	
	#ifdef DEBUG
		printf("Command line args processed\n");
	#endif

	sock = createSocket();
	master.listen_port = bindSocket(sock,master.listen_port,master.ip_addr);
	listenSocket(sock);

	#ifdef DEBUG
		printf("Master setup\n");
		printf("Master details are\nid: %d\nIP addr: %s\nListen Port: %d\n",master.id,master.ip_addr,master.listen_port);
	#endif
	printf("Potato Master on %s Port %d\n",master.ip_addr,master.listen_port);
	printf("Players = %d\nHops = %d\n",num_of_players,p.hops);
}

void print_ring()
{
	#ifdef DEBUG
		int i;
		for(i=0;i<num_of_players;i++)
			printf("Player %d\nIP address: %s\nListen Port: %d\n",player_list[i].id,player_list[i].ip_addr,player_list[i].listen_port);
	#endif
}

void setupNetwork()
{
	int i,ret;
	char *a, *b, *c;
	char msg[MAXLEN];
	for(i=0;i<num_of_players;i++)
	{
		player_list[i].id = i+1;

		player_list[i].conn_port = acceptConnection(sock);
		memset(msg,0,MAXLEN);	
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
		
		handleMessage(i,msg);
		printf("Player %d is on %s\n",player_list[i].id,player_list[i].ip_addr);
	
	}

	#ifdef DEBUG	
		printf("All players joined\n");
	#endif

	shutdown(sock,SHUT_RDWR);
	close(sock);
	for(i=0;i<num_of_players;i++)
	{
		sprintf(msg,"INFO\n%d\n%d\n%s\n%d\n",i+1,player_list[(i+1)%num_of_players].id,player_list[(i+1)%num_of_players].ip_addr,player_list[(i+1)%num_of_players].listen_port);
		if(send(player_list[i].conn_port,msg,strlen(msg),0)==-1)
		{
			printf("Sending failed\n");
			exit(-1);
		}
		#ifdef DEBUG
			printf("INFO message sent to player %d\n",i+1);	
		#endif
		FD_SET(player_list[i].conn_port,&readset);
		
	}

	#ifdef DEBUG
		printf("The ring is\n");
		print_ring();
	#endif	
}

int networkSetup()
{
	int i;
	for(i=0;i<num_of_players;i++)
		if(network_setup[i] == 0)
			return FALSE;
	printf("All players present");
	return TRUE;
}

int getMaxFd()
{
	int i,max;
	max = player_list[0].conn_port;
	for(i=1;i<num_of_players;i++)
		if(player_list[i].conn_port > max)
			max = player_list[i].conn_port;
	return max;
}

void wait_for_message()
{
	int i;
	char msg[MAXLEN];
	char *a,*b;
	int n,ret;
	int nfds;
	int flag = 1;
	nfds = getMaxFd()+1;
	#ifdef DEBUG
		printf("Waiting for message\nnfds is %d\n",nfds);	
	#endif

	while(flag)
	{
		for(i=0;i<num_of_players;i++)
			FD_SET(player_list[i].conn_port,&readset);
                select(nfds,&readset,NULL,NULL,NULL);
		
	#ifdef DEBUG
		printf("Some file descriptor is ready to read\n");
	#endif

                for(i=0;i<num_of_players;i++)
			if(FD_ISSET(player_list[i].conn_port,&readset))
				break;

		memset(msg,0,MAXLEN);
		if(recv(player_list[i].conn_port, msg, MAXLEN, 0) == -1)
		{
			printf("Receive error! \n");
			exit(-1);
		}
		flag = handleMessage(i,msg);		
		for(i=0;i<num_of_players;i++)
			FD_CLR(player_list[i].conn_port,&readset);
		FD_ZERO(&readset);
	}
	
}


