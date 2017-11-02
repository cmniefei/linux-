#include "pub.h"

void run(int port);


int main(int arg, char *args[])
{
	if (arg < 2)//如果没有参数，main函数退出
	{
		printf("usage:server port\n");
		return EXIT_SUCCESS;
	}

	int iport = atoi(args[1]);//将第一个参数转化为端口号
	if (iport == 0)
	{
		printf("port %d is invalid\n", iport);
		return EXIT_SUCCESS;
	}
	run(iport);
	return EXIT_SUCCESS;
}

