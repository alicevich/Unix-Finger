// Alice Vichitthavong
// CPSC 460 
// Project 1: finger client/server
// 1/20/14

// fingerclient.c

#include <stdio.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h> // socket funcs
#include <netinet/in.h> // sockaddr_in
#include <arpa/inet.h> // htons, inet_pton
#include <unistd.h> 

int main(int argc, char* argv[])
{
	char *user;
	char *hostname;
	//Retrieve username and hostname from commandline
	if(argc == 2) {
		user = malloc(1+ strlen(argv[1])); 
		if(!user) {
			printf("Error: unable to get username\n");
			return -1;
		}
		strcpy(user, argv[1]);
		hostname = strrchr(user, '@'); 
	} else {
		printf("Error: invalid number of arguments\n");
		return -1;
	}
	if(!hostname) {
		printf("Error: hostname is invalid\n");
		return -1; 
	}
	*hostname++ = '\0';
	
	//Create socket
	int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sock < 0) {
		printf("Error: Cannot create socket\n");
		return -1; 
	}
	
	//Establish connection:
	//Set address
	int port = 10095;
	struct sockaddr_in serverAddress;
	struct hostent *host;
	host = gethostbyname(hostname); 
	if(!host) {
		printf("Error: unknown host\n");
		close(sock);
		return -1; 
	}
	
	//Use bind to set an address and port number for connection
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(0);
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	int status = bind(sock, (struct sockaddr *) &serverAddress, sizeof(serverAddress));
	if(status < 0) {
		printf("Error: cannot bind\n");
		close(sock);
		return -1; 
	}
	
	//Use host to set the destination address
	serverAddress.sin_port = htons(port);
	memcpy(&serverAddress.sin_addr, host->h_addr, host->h_length);
	
	//Connect to server
	 status = connect(sock, (struct sockaddr *) &serverAddress, sizeof(serverAddress));
	if(status < 0) {
		printf("Error: cannot connect to host\n");
		close(sock);
		return -1; 
	}
	
	//Communicate	
	int length = strlen(user);
	write(sock, &length, sizeof(int)); //send length of username to server;
	write(sock, user, length); //send username to server
	
	//Display results from the finger command
	int count;
	char buffer[1024];
	while((count = read(sock, buffer, sizeof(buffer))) > 0) {
		close(sock);
		write(1,buffer, count);
		
	}		
	
	//Close Socket
	close(sock);
	return 0;
}
