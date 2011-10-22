#include<stdio.h>
int main(int argc, char *argv[])
{
	initMaster(argc,argv);
	setupNetwork();
	sendPotato();	
	return 0;
}
