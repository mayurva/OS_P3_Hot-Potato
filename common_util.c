#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <ifaddrs.h>
#include <net/if.h>
#include<sys/time.h>
#include"potato.h"

int getRandom(int lower,int upper)
{
	struct timeval tv;
	int seed;
	gettimeofday(&tv,NULL);
	seed = tv.tv_sec+tv.tv_usec;
	srand(seed);
	return ((rand()%(upper-lower))+lower);
}

player populatePublicIp(player p)
{

	struct ifaddrs *myaddrs, *ifa;
	void *in_addr;
	char buf[64], intf[128];

	strcpy(p.iface_name, "");

	if(getifaddrs(&myaddrs) != 0) {
		printf("getifaddrs failed! \n");
		exit(-1);
	}

	for (ifa = myaddrs; ifa != NULL; ifa = ifa->ifa_next) {

		if (ifa->ifa_addr == NULL)
			continue;

		if (!(ifa->ifa_flags & IFF_UP))
			continue;

		switch (ifa->ifa_addr->sa_family) {
        
			case AF_INET: { 
				struct sockaddr_in *s4 = (struct sockaddr_in *)ifa->ifa_addr;
				in_addr = &s4->sin_addr;
				break;
			}

			case AF_INET6: {
				struct sockaddr_in6 *s6 = (struct sockaddr_in6 *)ifa->ifa_addr;
				in_addr = &s6->sin6_addr;
				break;
			}

			default:
				continue;
		}

		if (inet_ntop(ifa->ifa_addr->sa_family, in_addr, buf, sizeof(buf))) {
			if ( ifa->ifa_addr->sa_family == AF_INET && strcmp(ifa->ifa_name, "lo")!=0 ) {
				#ifdef DEBUG
					printf("Player is binding to %s interface\n", ifa->ifa_name);
				#endif
				sprintf(p.ip_addr, "%s", buf);
				sprintf(p.iface_name, "%s", ifa->ifa_name);
			}
		}
	}

	freeifaddrs(myaddrs);
	
	if ( strcmp(p.iface_name, "") == 0 ) {
		printf("Either no Interface is up or you did not select any interface ..... \nmaster Exiting .... \n\n");
		exit(0);
	}
	#ifdef DEBUG
		printf("\n\nMy public interface and IP is:  %s %s\n\n", p.iface_name, p.ip_addr);
	#endif
	return p;
}

void listenSocket(int soc)
{
	#ifdef DEBUG
		printf("Listen for connection\n");
	#endif

        if(listen(soc,10) == -1) {
                printf("listen error\n");
                exit(-1);
        }
}

int acceptConnection(int soc)
{
	int conn_port;
	struct sockaddr_in sock_client;
	int slen = sizeof(sock_client);
	
	if ((conn_port = accept(soc, (struct sockaddr *) &sock_client, &slen)) == -1) {
		printf("accept call failed! \n");
		exit(-1);
	}
	
	#ifdef DEBUG
		printf("Connection accepted\n");
	#endif
	
	return conn_port;
}

int createConnection(player_tracker p)
{
        struct sockaddr_in sock_client;
        int slen = sizeof(sock_client);
        int ret;

        memset((char *) &sock_client, 0, sizeof(sock_client));
	
	#ifdef DEBUG
	        printf("Connecting to a process\nIP Addr: %s\nport: %d\n",p.ip_addr,p.listen_port);
	#endif

        sock_client.sin_family = AF_INET;
        sock_client.sin_port = htons(p.listen_port);
        sock_client.sin_addr.s_addr = inet_addr(p.ip_addr);

        ret = connect(p.conn_port, (struct sockaddr *) &sock_client, slen);
        if (ret == -1) {
                printf("Connect failed! Check the IP and port number of the Server! \n");
                exit(-1);
        }
	
	#ifdef DEBUG
        	printf("Connected to the process\nProcess id %d\n",p.id);
	#endif
}

int createSocket()
{
	int soc;
	if ((soc= socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		printf("error in socket creation\n");
		exit(-1);
	}
	#ifdef DEBUG
		printf("Socket created\n");
	#endif
	return soc;
}

int bindSocket(int soc, int listen_port, char ip_addr[])
{
	struct sockaddr_in sock_server;	
	memset((char *) &sock_server, 0, sizeof(sock_server));
	sock_server.sin_family = AF_INET;
	sock_server.sin_port = htons(listen_port);
	sock_server.sin_addr.s_addr = inet_addr(ip_addr);
	while (bind(soc, (struct sockaddr *) &sock_server, sizeof(sock_server)) == -1) {
		listen_port = getRandom(1024,65636);
		sock_server.sin_port = htons(listen_port);
	}

	#ifdef DEBUG
		printf("Player listens on port %d\n",listen_port);
		printf("Listen Socket created\n");
	#endif

	return listen_port;
}


char* receiveMessage(player_tracker p)
{
}

//reduce the buffer length so that message needs to be sent in chunks
void sendMessage(player_tracker p,char *message)
{
        char sendbuf[MAXLEN];
        strcpy(sendbuf,message);
        if(send(p.conn_port, sendbuf, MAXLEN, 0) == -1){
                printf("send failed\n ");
                exit(-1);
        }

	#ifdef DEBUG
	        printf("%s message sent\n",message);
	#endif
}

