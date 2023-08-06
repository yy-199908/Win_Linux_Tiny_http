#include <iostream>
#include "YYServer.h"
using namespace std;
int main(int argc, char* argv[])
{
	int port = 80;
	if (argc == 2 )
	{
		port = atoi(argv[1]);
	}
	YYServer yy;
	int sock_server = -1;
	sock_server = yy.startup((unsigned short*)&port);
	if (sock_server != -1)
	{
		printf("http start, port: %d\n", port); 
	}
	yy.Main();
}