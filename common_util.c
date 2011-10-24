#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <ifaddrs.h>
#include <net/if.h>

#include"potato.h"

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
				printf("Player is binding to %s interface\n", ifa->ifa_name);
				/*scanf("%s", intf);
				if ( strcmp(intf, "n") == 0 )
					continue;*/
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

	printf("\n\nMy public interface and IP is:  %s %s\n\n", p.iface_name, p.ip_addr);
	return p;
}

int createConnection(player_tracker p)
{
        struct sockaddr_in sock_client;
        int slen = sizeof(sock_client);
        int ret;

        memset((char *) &sock_client, 0, sizeof(sock_client));

        printf("Connecting to a process\nIP Addr: %s\nport: %d\n",p.ip_addr,p.listen_port);

        sock_client.sin_family = AF_INET;
        sock_client.sin_port = htons(p.listen_port);
        sock_client.sin_addr.s_addr = inet_addr(p.ip_addr);

        ret = connect(p.conn_port, (struct sockaddr *) &sock_client, slen);
        if (ret == -1) {
                printf("Connect failed! Check the IP and port number of the Sever! \n");
                exit(-1);
        }
        printf("Connected to the process\nProcess id %d",p.id);
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
                printf("send failed ");
                exit(-1);
        }

        printf("%s message sent\n",message);
}

