#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define BUFSIZE 262144  //256k



static char * user;
void * recvsocket(void *arg) {
        int client_st = *(int*) arg;
        char str[BUFSIZE];
        while (1) {
			memset(&str, 0, sizeof(str));
			if (recv(client_st, &str, sizeof(str), 0) <= 0) {
					break;
			}
			char *sendermsg = strtok(str,",");
			char *datamsg = strtok(NULL,",");
			
			strtok(sendermsg,"=");
			char *sender = strtok(NULL,"=");
			
			strtok(datamsg,"=");
			char *data = strtok(NULL,"=");
			
			printf("%s:%s",sender,data);
			
			//printf("%s", str);
        }
        return NULL;
}

void * sendsocket(void *arg) {
        int client_st = *(int*) arg;
        char str[BUFSIZE];
		char deststr[BUFSIZE];
        while (1) {
                memset(&str, 0, BUFSIZE);
				memset(&deststr, 0, BUFSIZE);
                read(STDIN_FILENO, &str, sizeof(str));
				int sendlen = strlen(str);
				if(sendlen==1)//说明只输入了enter
				{
					printf("send can not be null\n");
					break;
				}
				str[sendlen]='\0';//取出最后的enter
				sprintf(deststr,"sender=%s,data=%s",user,str);
				
                if (send(client_st, &deststr, strlen(deststr), 0) <= 0) {
                    break;
                }
        }
        return NULL;
}
int main(int arg, char *args[]) {
        if(arg<4){
                return EXIT_FAILURE;
        }
        int sk = socket(AF_INET, SOCK_STREAM, 0);//
        if (sk == -1) {
                printf("socket failed %s\n", strerror(errno));
                return EXIT_FAILURE;
        }

        int port = atoi(args[2]);
		user = args[3];
		
        struct sockaddr_in addr;//ip address
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;//tcp/ip协议
        addr.sin_port =htons(port);////将本机的short数据转化为网络上的short数据
        addr.sin_addr.s_addr = inet_addr(args[1]);
        //客户端开始建立连接
        if (connect(sk, (struct sockaddr*) &addr, sizeof(addr)) == -1) {
                printf("connect error :%s\n", strerror(errno));
                return EXIT_FAILURE;
        }
        //开启两个线程，一个发送消息，一个监听消息
        pthread_t th1, th2;
        pthread_create(&th1, NULL, recvsocket, &sk);
        pthread_create(&th2, NULL, sendsocket, &sk);

        pthread_join(th1, NULL);
        //关闭客户端文件描述符
        close(sk);

        return EXIT_SUCCESS;
}
