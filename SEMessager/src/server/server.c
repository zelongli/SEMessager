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
	unsigned char message[MAX_MESSAGELEN];
	client_arg* args = (client_arg*)arg;
	int sockfd = args->client_sockfd;
	int numbytes;
	if((numbytes = recv(sockfd,message, MAX_MESSAGELEN,0))==-1){
		perror("recv error");
		exit(1);
	}

	message[numbytes] = '\0';
	printf("Message from client %s\n", message);

	long hash =0;

	for(int i = 0; i< numbytes; ++i){
		hash+=message[i];
	}
//	printf("hashbalue %d\n",hash);
	unsigned char hashbyte[4];
	hashbyte[0] = (hash >> 24) & 0xFF;
	hashbyte[1] = (hash >> 16) & 0xFF;
	hashbyte[2] = (hash >> 8) & 0xFF;
	hashbyte[3] = hash & 0xFF;

	unsigned char key[4];

	key[0] = 192;
	key[1] = 168;
	key[2] = 1;
	key[3] = 1;

	for(int i = 0; i< 4; ++i){
		hashbyte[i] ^= key[i];
	}

	if((numbytes = recv(sockfd,message, MAX_MESSAGELEN,0))==-1){
		perror("recv error");
		exit(1);
	}

	message[numbytes] = '\0';
	unsigned char *result =1;
	for(int i =0 ; i< 4 ; ++i){
//		printf("got server%d,client%d \n",hashbyte[i],message[i]);
		if(hashbyte[i] != message[i]){
			*result = 0;
		}
	}

//	unsigned char reply[MAX_MESSAGELEN];
	if(result){
		char reply[1];
		memset(reply,1,1);
		printf("send result  %d\n", reply[0]);
		// sprintf(reply,result,1);
		send(sockfd,reply,1,0);
	}else{
		char reply[1];
		memset(reply,0,1);
		printf("send result  %d\n", reply[0]);
		// sprintf(reply,result,1);
		send(sockfd,reply,1,0);
	}
//	unsigned char resulttosend[1024];
//	int sendlen = sprintf(resulttosend,(char *)&result, send)
//	printf("Signature received client %d\n",result);
//	send(sockfd,result,1,0);
}

int main()
{
	int server_sockfd;
	int server_len;
	struct sockaddr_in server_address;
	int result;

	server_sockfd = socket(AF_INET,SOCK_STREAM,0);
//	printf("im listening \n");
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