#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <ifaddrs.h>
#include <net/if.h>

#include"potato.h"

player populate_public_ip(player p)
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
				printf("\nDo you want master to bind to %s interface?(y/n): ", ifa->ifa_name);
				scanf("%s", intf);
				if ( strcmp(intf, "n") == 0 )
					continue;
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
