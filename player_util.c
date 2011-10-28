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
int termflag;
potato p;

pthread_t neighbor_thread[2];
pthread_t master_thread;

int initPlayer(char *argv[])
{
	struct sockaddr_in sock_server;

	master.id = 0;
	strcpy(master.ip_addr,argv[1]);
	//printf("%s\n",argv[2]);
	master.listen_port = atoi(argv[2]);

	#ifdef DEBUG
		printf("Master port num is %d\n",master.listen_port);
		printf("Command line args processed\n");
	#endif
	
	me = populatePublicIp(me);
	
	if ((sock= socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		printf("error in socket creation\n");
		exit(-1);
	}

	me.listen_port= master.listen_port;

        memset((char *) &sock_server, 0, sizeof(sock_server));
        sock_server.sin_family = AF_INET;
        sock_server.sin_port = htons(me.listen_port);
        sock_server.sin_addr.s_addr = inet_addr(me.ip_addr);
	while (bind(sock, (struct sockaddr *) &sock_server, sizeof(sock_server)) == -1) {
                me.listen_port = (rand() % (65535-1024)) + 1024;
                sock_server.sin_port = htons(me.listen_port);
        }

	printf("Player listens on port %d\n",me.listen_port);
	
	#ifdef DEBUG
		printf("Listen Socket created\n");
	#endif

        if ((master.conn_port = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
                printf("error in socket creation\n");
                exit(-1);
        }

	if ((neighbor[1].conn_port = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		printf("error in socket creation\n");
		exit(-1);
	}
	

//	termflag = 0;

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

void* leftNeighborConn(void *args)
{
	struct sockaddr_in sock_client;
	int slen = sizeof(sock_client);
	char msg[MAXLEN];
	char *a;	

	#ifdef DEBUG
		printf("Listen for connection\n");
	#endif

        if(listen(sock,10) == -1) {
                printf("listen error\n");
                exit(-1);
        }

	if ((neighbor[0].conn_port = accept(sock, (struct sockaddr *) &sock_client, &slen)) == -1) {
		printf("accept call failed! \n");
		exit(-1);
	}
	
	#ifdef DEBUG
		printf("Connection accepted\n");
	#endif
	
	if(recv(neighbor[0].conn_port, msg, MAXLEN, 0) == -1)
        {
                printf("Receive error! \n");
                exit(-1);
        }
        a = strtok(msg,"\n");
        if(strcmp(a,"LEFT")==0)
        {
		a = strtok(NULL,"\n");
		neighbor[0].id = atoi(a);
		a = strtok(NULL,"\n");
		strcpy(neighbor[0].ip_addr,a);
	
        }

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
	fd_set readset;
	
	#ifdef DEBUG
		printf("Inside wait for msg\n");
	#endif

	FD_SET(master.conn_port,&readset);
	FD_SET(neighbor[0].conn_port,&readset);
	FD_SET(neighbor[1].conn_port,&readset);

	ndfs = master.conn_port>neighbor[0].conn_port?master.conn_port:neighbor[0].conn_port;
	ndfs = ndfs>neighbor[1].conn_port?ndfs:neighbor[1].conn_port;
	ndfs++;
	printf("ndfs is %d\n",ndfs);

	while(1)
	{
/*		FD_CLR(master.conn_port,&readset);
		FD_CLR(neighbor[0].conn_port,&readset);
		FD_CLR(neighbor[1].conn_port,&readset);*/
		select(ndfs,&readset,NULL,NULL,NULL);
		{
			#ifdef DEBUG
				printf("Some descriptor is ready to recieve\n");
			#endif

			if(FD_ISSET(master.conn_port,&readset))
			{
				#ifdef DEBUG
					printf("message from master\n");	
				#endif

				if(recv(master.conn_port, msg, MAXLEN, 0) == -1)
				{
					printf("Receive error! \n");
					exit(-1);
				}
				a = strtok_r(msg,"\n",&b);
				if(strcmp(a,"TERM")==0)
		        	{
					#ifdef DEBUG
						printf("Term Signal Received\n");
					#endif
					break;
				}
				else if(strcmp(a,"POTA")==0)
				{
					a = strtok_r(NULL,"\n",&b);
					p.hops = atoi(a);
	
					#ifdef DEBUG
						printf("\nReceived potato from player %d\nhop value is %d\n\n",neighbor[i].id,p.hops);
					#endif

					p.identities = malloc(MAXLEN*sizeof(char));
					sprintf(p.identities,"%s\n%d\n",b,me.id);
					sprintf(msg,"POTA\n%d\n%s",p.hops,p.identities);
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
						p.hops--;
						j=rand()%2;	
				
						#ifdef DEBUG
							printf("Sending potato to player %d\n",neighbor[j].id);
						#endif
	
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
			a = strtok_r(msg,"\n",&b);
			if(strcmp(a,"POTA")==0)
			{
				a = strtok_r(NULL,"\n",&b);
				p.hops = atoi(a);

				#ifdef DEBUG
					printf("\nReceived potato from player %d\nhop value is %d\n\n",neighbor[i].id,p.hops);
				#endif

				p.identities = malloc(MAXLEN*sizeof(char));
				sprintf(p.identities,"%s\n%d\n",b,me.id);
				sprintf(msg,"POTA\n%d\n%s",p.hops,p.identities);
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
					p.hops--;
					j=rand()%2;	
				
					#ifdef DEBUG
						printf("Sending potato to player %d\n",neighbor[j].id);
					#endif
	
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
		}
		
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
	a = strtok(msg,"\n");
	if(strcmp(a,"INFO")==0)
	{
		a=strtok(NULL,"\n");
		me.id = atoi(a);
		a=strtok(NULL,"\n");
		neighbor[1].id = atoi(a);
		a=strtok(NULL,"\n");
		strcpy(neighbor[1].ip_addr,a);
		a=strtok(NULL,"\n");
		neighbor[1].listen_port = atoi(a);
		#ifdef DEBUG
			printf("Right neighbor (id %d) info updated for player %d\n",neighbor[1].id,me.id);
		#endif
	}

	createConnection(neighbor[1]);
			
	sprintf(msg,"LEFT\n%d\n%s\n",me.id,me.ip_addr);

        #ifdef DEBUG
                printf("Message to be sent is %s\n",msg);
        #endif

        if(send(neighbor[1].conn_port,msg,strlen(msg),0)==-1)
        {
                printf("Sending failed\n");
                exit(-1);
        }

	
        if(pthread_join(neighbor_thread[0],NULL) !=0 ) {
                printf("Join failed\n");
                exit(-1);
        }

}
void printPlayerInfo()
{
	printf("\nThis is player %d\n",me.id);
	printf("Left neighbor is %d\n",neighbor[0].id);
	printf("Right Neighbor is %d\n",neighbor[1].id);
}



