#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/stat.h>
#include <time.h>
#include <pthread.h>
#define MAX_THREAD 1024 //how many pending conenections queue will hold
#define MAX_MESSAGELEN 1023
typedef struct 
{
	int client_sockfd;
}client_arg;

void get_ip_address(unsigned long address, char*ip)
{
	sprintf(ip,"%d.%d.%d.%d",(int)address>>24,(int)(address&0xFF000000)>>24,(int)(address&0xFF00)>>24,(int)address&0xFF);
}

void* reply(void* arg)
{
	char message[MAX_MESSAGELEN];
	client_arg* args = (client_arg*)arg;
	printf("Message from client nothing");
	int sockfd = args->client_sockfd;
	int numbytes;
	if((numbytes = recv(sockfd,message, MAX_MESSAGELEN,0))==-1){
		perror("recv error");
		exit(1);
	}

	message[numbytes] = '\0';
	printf("Message from client %s\n",message);

	// long hash =0;

	// for(int i = 0; i< numbytes; ++i){
	// 	hash+=message[i];
	// }

	// unsigned char bytes[4];

	// bytes[0] = (hash >> 24) & 0xFF;
	// bytes[1] = (hash >> 16) & 0xFF;
	// bytes[2] = (hash >> 8) & 0xFF;
	// bytes[3] = hash & 0xFF;

	// for(int i = 0; i< 4; ++i){
	// 	bytes[i]
	// }

	// if((numbytes = recv(sockfd,message, MAX_MESSAGELEN,0))==-1){
	// 	perror("recv error");
	// 	exit(1);
	// }

	// message[numbytes] = '\0';
	// printf("Signature received client %s\n",message);
}

int main()
{
	int server_sockfd;
	int server_len;
	struct sockaddr_in server_address;
	int result;

	server_sockfd = socket(AF_INET,SOCK_STREAM,0);
	printf("im listening \n");
	server_address.sin_family=AF_INET;
	server_address.sin_addr.s_addr=htonl(INADDR_ANY);
	server_address.sin_port=htons(6666);
	server_len=sizeof(server_address);

	bind(server_sockfd,(struct sockaddr*)&server_address, server_len);

	if (listen(server_sockfd, MAX_THREAD) == -1) {
        perror("listen error");
        exit(1);
    }

	while(1)
	{
		int client_sockfd;
		struct sockaddr_in client_address;
		int client_len;
		char ip_address[16];
		client_arg* args;
		client_len = sizeof(client_address);

		client_sockfd=accept(server_sockfd,(struct sockaddr*)&client_address,(socklen_t*)&client_len);

		args=(client_arg*)malloc(sizeof(client_arg));
		args->client_sockfd=client_sockfd;

		get_ip_address(ntohl(client_address.sin_addr.s_addr),ip_address);
		printf("getting connect from %s \n", ip_address);

		// thread
		pthread_t client_thread;

		pthread_attr_t thread_attr;
		int res;
		res = pthread_attr_init(&thread_attr);

		if(res != 0)
		{
			perror("Attribute creation failed");
			free(args);
			close(client_sockfd);
			continue;
		}

		res=pthread_attr_setdetachstate(&thread_attr,PTHREAD_CREATE_DETACHED);
		if(res != 0)
		{
			perror("Setting detached attribute failed");
			free(args);
			close(client_sockfd);
			continue;
		}

		res=pthread_create(&client_thread,&thread_attr,reply, (void *)args);
		if(res != 0)
		{
			perror("Thread creation failed");
			free(args);
			close(client_sockfd);
			continue;
		}
		pthread_attr_destroy(&thread_attr);
	}
	return 0;

}