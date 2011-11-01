#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include<pthread.h>
#include <sys/select.h>

#include"potato.h"
#include"player.h"

player_tracker master;
player me;
player_tracker neighbor[2];
int sock;
potato p;
fd_set readset;

pthread_t left_thread;

void processPotato(char p_string[])
{
	char *a,*b;
	a = strtok_r(p_string,"\n",&b);
	p.hops = atoi(a);
	char msg[MAXLEN];
	memset(msg,0,MAXLEN);
	
	#ifdef DEBUG
		printf("\nReceived potato from master with hops %d\n",p.hops);
	#endif

	p.identities = malloc(MAXLEN*sizeof(char));
	memset(p.identities,0,MAXLEN*sizeof(char));

	if(b==NULL)
		sprintf(p.identities,"%d\n",me.id);
	else
		sprintf(p.identities,"%s%d\n",b,me.id);

	memset(b,0,strlen(b));
	p.hops--;
	sprintf(msg,"POTA\n%d\n%s",p.hops,p.identities);
	#ifdef DEBUG
		printf("\nPotato is\n%s\n",p.identities);
	#endif
	if(p.hops == 0)
	{
		printf("I'm it\n");

		#ifdef DEBUG
			printf("Sending potato to the master\n");
		#endif
			
	        if(send(master.conn_port,msg,strlen(msg),0)==-1)
		{
			printf("Sending failed\n");
			exit(-1);
		}
	
		#ifdef DEBUG
			printf("Potato sent to the master\n");
		#endif
	}
	else
	{
		int j;
		j=getRandom(0,2);	

		printf("Sending potato to player %d\n",neighbor[j].id);
	
		if(send(neighbor[j].conn_port,msg,strlen(msg),0)==-1)
		{
			printf("Sending failed\n");
			exit(-1);
		}
                                
		#ifdef DEBUG
			printf("Potato sent to player %d\n",neighbor[j].id);
		#endif
	}
	free(p.identities);
}

int handleMessage(char msg[])
{
	if(msg)
	{
		char *a,*b;
		b = NULL;
		a = strtok_r(msg,"\n",&b);
        	if(strcmp(a,"LEFT")==0)
        	{
			a = strtok_r(NULL,"\n",&b);
			neighbor[0].id = atoi(a);
			a = strtok_r(NULL,"\n",&b);
			strcpy(neighbor[0].ip_addr,a);
		}
		else if(strcmp(a,"TERM")==0)
	       	{
			#ifdef DEBUG
				printf("Term Signal Received\n");
			#endif
			shutdown(master.conn_port,SHUT_RDWR);
			close(master.conn_port);
			shutdown(neighbor[0].conn_port,SHUT_RDWR);
			close(neighbor[0].conn_port);
			shutdown(neighbor[1].conn_port,SHUT_RDWR);
			close(neighbor[1].conn_port);
			return 0;
		}
		else if(strcmp(a,"INFO")==0)
		{
			a=strtok_r(NULL,"\n",&b);
			me.id = atoi(a);
			a=strtok_r(NULL,"\n",&b);
			neighbor[1].id = atoi(a);
			a=strtok_r(NULL,"\n",&b);
			strcpy(neighbor[1].ip_addr,a);
			a=strtok_r(NULL,"\n",&b);
			neighbor[1].listen_port = atoi(a);
			#ifdef DEBUG
				printf("Right neighbor (id %d) info updated for player %d\n",neighbor[1].id,me.id);
			#endif
		}

		else if(strcmp(a,"POTA")==0)
			processPotato(b);
	}
	return 1;
}

int initPlayer(char *argv[])
{
	master.id = 0;
	strcpy(master.ip_addr,argv[1]);
	master.listen_port = atoi(argv[2]);

	#ifdef DEBUG
		printf("Master port num is %d\n",master.listen_port);
		printf("Command line args processed\n");
	#endif
	
	me = populatePublicIp(me);
	
	sock = createSocket();
	me.listen_port= bindSocket(sock,master.listen_port,master.ip_addr);

        master.conn_port = createSocket();
	neighbor[1].conn_port = createSocket();

	#ifdef DEBUG
		printf("Connection Socket created\n");
		printf("Player initialized\n");
	#endif
}

void* leftNeighborConn(void *args)
{

	char msg[MAXLEN];
	char *a;	

	listenSocket(sock);
	neighbor[0].conn_port = acceptConnection(sock);
	
	if(recv(neighbor[0].conn_port, msg, MAXLEN, 0) == -1)
        {
                printf("Receive error! \n");
                exit(-1);
        }
	
	handleMessage(msg);
	memset(msg,0,MAXLEN);
	FD_SET(neighbor[0].conn_port,&readset);
	shutdown(sock,SHUT_RDWR);
	close(sock);
	#ifdef DEBUG
		printf("Updated left neighbour info\n");
	#endif
}

void wait_for_message()
{
	int i,j;
	char msg[MAXLEN];
	char *a,*b;
	int ndfs;
	int flag = 1;
	#ifdef DEBUG
		printf("Inside wait for msg\n");
	#endif


	ndfs = master.conn_port>neighbor[0].conn_port?master.conn_port:neighbor[0].conn_port;
	ndfs = ndfs>neighbor[1].conn_port?ndfs:neighbor[1].conn_port;
	ndfs++;

	#ifdef DEBUG
		printf("Master fd is %d\nleft neighbor fd is %d\nRgiht neighbor fd is %d\nndfs is %d\n",master.conn_port,neighbor[0].conn_port,neighbor[1].conn_port,ndfs);
	#endif

	while(flag)
	{
		FD_SET(master.conn_port,&readset);
		FD_SET(neighbor[0].conn_port,&readset);
		FD_SET(neighbor[1].conn_port,&readset);
		select(ndfs,&readset,NULL,NULL,NULL);
		{
			#ifdef DEBUG
				printf("Some descriptor is ready to recieve\n");
			#endif
			memset(msg,0,MAXLEN);
			if(FD_ISSET(master.conn_port,&readset))
			{
				if(recv(master.conn_port, msg, MAXLEN, 0) == -1)
				{
					printf("Receive error! \n");
					exit(-1);
				}
				flag = handleMessage(msg);	
				FD_CLR(master.conn_port,&readset);
				FD_CLR(neighbor[0].conn_port,&readset);
				FD_CLR(neighbor[1].conn_port,&readset);
				FD_ZERO(&readset);
				continue;
			}
			else if(FD_ISSET(neighbor[0].conn_port,&readset))
			{
				#ifdef DEBUG
					printf("message from left\n");	
				#endif
				i=0;
			}	
			
			else if(FD_ISSET(neighbor[1].conn_port,&readset))
			{
				#ifdef DEBUG
					printf("message from right\n");	
				#endif
				i=1;
			}
			if(recv(neighbor[i].conn_port, msg, MAXLEN, 0) == -1)
			{
				printf("Receive error! \n");
				exit(-1);
			}

			flag = handleMessage(msg);
		}
		FD_CLR(master.conn_port,&readset);
		FD_CLR(neighbor[0].conn_port,&readset);
		FD_CLR(neighbor[1].conn_port,&readset);
		FD_ZERO(&readset);
	}
}

void setupNetwork()
{
	char msg[MAXLEN];
	createConnection(master);
	char *a;
	#ifdef DEBUG
		printf("Connected to master\n");
	#endif

	sprintf(msg,"JOIN\n%s\n%d\n",me.ip_addr,me.listen_port);

	#ifdef DEBUG
		printf("Message to be sent is %s\n",msg);
	#endif

	if(send(master.conn_port,msg,strlen(msg),0)==-1)
	{
		printf("Sending failed\n");
		exit(-1);
	}

	#ifdef DEBUG
		printf("JOIN message sent\n");
	#endif
	if(recv(master.conn_port, msg, MAXLEN, 0) == -1)	
	{
		printf("Receive error! \n");
		exit(-1);
	}
	handleMessage(msg);

	printf("Connected as player %d\n",me.id);	

	createConnection(neighbor[1]);
	memset(msg,0,MAXLEN);			
	sprintf(msg,"LEFT\n%d\n%s\n",me.id,me.ip_addr);

        #ifdef DEBUG
                printf("Message to be sent is %s\n",msg);
        #endif

        if(send(neighbor[1].conn_port,msg,strlen(msg),0)==-1)
        {
                printf("Sending failed\n");
                exit(-1);
        }
	FD_SET(neighbor[1].conn_port,&readset);

	
        if(pthread_join(left_thread,NULL) !=0 ) {
                printf("Join failed\n");
                exit(-1);
        }

	#ifdef DEBUG
		printf("thread joined\n");
	#endif
	
	sprintf(msg,"CONN\n%d\n",me.id);
	if(send(master.conn_port,msg,strlen(msg),0)==-1)
        {
                printf("Sending failed\n");
                exit(-1);
        }
	memset(msg,0,MAXLEN);

	#ifdef DEBUG
		printf("Message sent to master is\n%s\n\n",msg);	
	#endif

	FD_SET(master.conn_port,&readset);
}
void printPlayerInfo()
{
	printf("\nThis is player %d\n",me.id);
	printf("Left neighbor is %d\n",neighbor[0].id);
	printf("Right Neighbor is %d\n",neighbor[1].id);
}



