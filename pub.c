#include "pub.h"

#define MAXCHAT_P 100
int sts[MAXCHAT_P]={0};//目前最多允许MAXCHAT_P个人加入在线聊天
int chatsum=0;

//添加一个聊天的人
int addchatpeople(int st)
{
	int i;
	for(i=0;i<MAXCHAT_P;i++)
	{
		
		if(sts[i]==0)
		{
			sts[i]=st;
			chatsum++;
			return 1;
		}
	}
	return 0;//满了
	
}
//一个聊天的退出了
void deletechatpeople(int st)
{
	int i;
	for(i=0;i<MAXCHAT_P;i++)
	{
		if(sts[i]==st)
		{
			sts[i]=0;	
			chatsum--;
		}
	}
}

//将聊天记录发给聊天室其他成员
void sendtocharroom(char * buf,int currst)
{
	int i;
	for(i=0;i<MAXCHAT_P;i++)
	{
		if(sts[i]!=0&&sts[i]!=currst)
		{
			send(sts[i], buf, strlen(buf), 0);
		}
	}
	
}


int socket_create(int port) //创建参数port指定端口号的server端socket
{
	int st = socket(AF_INET, SOCK_STREAM, 0); //创建TCP Socket
	int on = 1;
	if (setsockopt(st, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1)
	{
		printf("setsockopt failed %s\n", strerror(errno));
		return 0;
	}
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(st, (struct sockaddr *) &addr, sizeof(addr)) == -1)
	{
		printf("bind port %d failed %s\n", port, strerror(errno));
		return 0;
	}
	if (listen(st, 300) == -1)
	{
		printf("listen failed %s\n", strerror(errno));
		return 0;
	}
	return st; //返回listen的socket描述符
}

int setnonblocking(int st) //将socket设置为非阻塞
{
	int opts = fcntl(st, F_GETFL);
	if (opts < 0)
	{
		printf("fcntl failed %s\n", strerror(errno));
		return 0;
	}
	opts = opts | O_NONBLOCK;
	if (fcntl(st, F_SETFL, opts) < 0)
	{
		printf("fcntl failed %s\n", strerror(errno));
		return 0;
	}
	return 1;
}


int socket_recv(int st)
{
	char recvbuf[200];
	memset(recvbuf, 0, sizeof(recvbuf));
	char *sendbuf = (char *)malloc(BUFSIZE);
	memset(sendbuf, 0, BUFSIZE);
	ssize_t rc = recv(st, recvbuf, sizeof(recvbuf), 0); //接收来自client socket发送来的消息
	
	if (rc > 0) //接收失败
	{
		sendtocharroom(recvbuf,st);	
	}
	free(sendbuf);
	return rc;
}

int socket_accept(int listen_st)
{
	struct sockaddr_in client_addr;
	int rv=0;
	socklen_t len = sizeof(client_addr);
	
	memset(&client_addr, 0, sizeof(client_addr));
	
	int client_st = accept(listen_st,  (struct sockaddr *)&client_addr, &len); //接收到来自client的socket连接

	if (client_st < 0)
	{
			printf("accept failed %s\n", strerror(errno));
	}
	else
	{
		printf("accept by %s\n", inet_ntoa(client_addr.sin_addr));
		
		rv = addchatpeople(client_st);
		char deststr[BUFSIZE];
		memset(deststr, 0, BUFSIZE);
		if(rv<=0)//超过最大聊天人数
		{
			sprintf(deststr,"sender=系统提示,data=聊天室已经满了！\n");
			send(client_st, deststr, strlen(deststr), 0);
			close(client_st);
			return -1;

		}else{
			sprintf(deststr,"sender=系统提示,data=当前 %d 人正在聊天 ！\n",chatsum);
			printf("%s\n",deststr);
			send(client_st, deststr, strlen(deststr), 0);
			
		}
		
	}	
	
	return client_st; //返回来自client端的socket描述符
}

void run(int port)
{
	int listen_st = socket_create(port); //创建参数port指定端口号的server端socket
	setnonblocking(listen_st); //把socket设置为非阻塞方式
	struct epoll_event ev, events[MAX_EVENT]; //声明epoll_event结构体的变量,ev用于注册事件,数组用于回传要处理的事件
	int epfd = epoll_create(MAX_EVENT); //生成用于处理accept的epoll专用的文件描述符

	ev.data.fd = listen_st; //设置与要处理的事件相关的文件描述符
	ev.events = EPOLLIN | EPOLLERR | EPOLLHUP; //设置要处理的事件类型

	epoll_ctl(epfd, EPOLL_CTL_ADD, listen_st, &ev); //注册epoll事件

	int st = 0;
	while (1)
	{
		int nfds = epoll_wait(epfd, events, MAX_EVENT, -1); //等待epoll事件的发生
		if (nfds == -1)
		{
			printf("epoll_wait failed %s\n", strerror(errno));
			break;
		}
		
		int i = 0;
		for (; i < nfds; i++)
		{
			if (events[i].data.fd < 0)
				continue;
			
			
			if (events[i].data.fd == listen_st) //监测到一个SOCKET用户连接到了绑定的SOCKET端口，建立新的连接。
			{
				st = socket_accept(listen_st);
				if (st >= 0)
				{
					
					setnonblocking(st); //将来自client端的socket描述符设置为非阻塞
					ev.data.fd = st;
					ev.events = EPOLLIN | EPOLLERR | EPOLLHUP; //设置要处理的事件类型
					epoll_ctl(epfd, EPOLL_CTL_ADD, st, &ev); //将来自client端的socket描述符加入epoll	
				}
				continue;
			}
			if (events[i].events & EPOLLIN) //socket收到数据
			{
				st = events[i].data.fd;
				if (socket_recv(st) <= 0)
				{
					deletechatpeople(st);//将这个人从聊天队列中踢出
					close(st);
					events[i].data.fd = -1;
				}
			}
			if (events[i].events & EPOLLERR) //socket错误。
			{
				st = events[i].data.fd;
				close(st);
				events[i].data.fd = -1;
			}

			if (events[i].events & EPOLLHUP) //socket被挂断。
			{
				st = events[i].data.fd;
				close(st);
				events[i].data.fd = -1;
			}
		}
	}
	close(epfd);
}