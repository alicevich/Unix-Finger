// Alice Vichitthavong
// CPSC 460 
// Project 1: finger client/server
// 1/20/14

// fingerserver.c

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <netdb.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/socket.h> // socket funcs
#include <netinet/in.h> // sockaddr_in
#include <arpa/inet.h> // htons, inet_pton
#include <unistd.h> 
#define FINGER_COMMAND "/usr/bin/finger"

typedef struct
{
	int clientSock;
} ThreadArgs;

void *threadMain(void *args)
{
	//Insures thread resources are deallocated on return
	pthread_detach(pthread_self());
	
	//Extract socket file descriptor from argument
	if (!args) {
		pthread_exit(0); 
	}	
	ThreadArgs *threadArgs = (ThreadArgs *)args;
	int clientSock = threadArgs->clientSock;
	free(threadArgs);
	
	//Communicate with client
	int length;
	int count = read(clientSock, &length, sizeof(int)); //read in length of username to 'length'
	char* buffer;
	if(length > 0) {
		buffer = (char *)malloc((length+1)*sizeof(char));
		buffer[length] = 0;
		count = read(clientSock, buffer, length); //read username and store to buffer.
		if(dup2(clientSock, 1) < 0 || dup2(clientSock, 2) < 0) {
			printf("dup2");
		}
	}
	
	//Close client socket
	close(clientSock);
	
	//Process finger command
	execl(FINGER_COMMAND, "finger", "--", *buffer ? buffer : NULL, NULL);
	free(buffer);
	printf("Error: Couldn't run finger");
	pthread_exit(0);
}

int main(int argc, char ** argv)
{
	//Create socket 
	int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sock < 0) {
		printf("Error: cannot create socket");
		return -1; 
	}
	
	//Bind port to socket
	int port = 10095;
	struct sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);
	
	int status = bind(sock, (struct sockaddr *) &address, sizeof(struct sockaddr_in));
	if(status < 0) {
		close(sock);
		printf("Error: cannot bind socket to port");
		return -1; 
	}

	//Set socket to listen
	status = listen(sock, 5);
	if(status < 0) {
		close(sock);
		printf("Error: cannot set socket to listen");
		return -1; 
	}
	
	printf("Ready and listening...\n");
	
	//Handling multiple clients(accept connection, communicate, close socket)
	while(1) { 
		//Accept connection from client
		struct sockaddr_in clientAddr;
		int addrLen = sizeof(clientAddr);
		int clientSock = accept(sock, (struct sockaddr *) &clientAddr, &addrLen);
		
		printf("connection from %s\n", inet_ntoa(clientAddr.sin_addr));
		
		//Create and initialize ThreadArgs struct
		ThreadArgs* args = (ThreadArgs*)malloc(sizeof(ThreadArgs));
		args->clientSock = clientSock;
		if(clientSock < 0) { 
			free(args);
		} 

		int processID = fork();
		if(!processID) {
			//Child process, so create client thread
			pthread_t threadID;
			status = pthread_create(&threadID, 0, threadMain, (void *) args);
			if(status != 0) {
				printf("Error creating thread");
			}
		}	
	}
	return 0;
}
