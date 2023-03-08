#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(){

	int clientSocket, ret;
	struct sockaddr_in serverAddr;
	char buffer[1024];
	char ipAddress[100];
	int port;
	clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(clientSocket < 0){
		printf("[-]Error in connection.\n");
		exit(1);
	}
	printf("[+]Client Socket is created.\n");
	memset(&serverAddr, '\0', sizeof(serverAddr));

	//working with 127.0.0.1 ip address
	printf("Write IP ADDRESS : \t");
	scanf("%s", &ipAddress[0]);

	//working with 4444 port
	printf("Write port number: \t");
	scanf("%d", &port);
	
	//type of socket created
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	serverAddr.sin_addr.s_addr = inet_addr(ipAddress);

	//connect to server
	ret = connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));

	//wait send message from server
	recv(clientSocket, buffer, 1024, 0);

	////Check if the limit is full
	if(strcmp(buffer, "true") == 0){ ret=-1; }
	if(ret < 0){
		printf("[-]Error in connection.\n");
		exit(1);
	}
	printf("[+]Connected to Server.\n");
	//keep communicating with server
	while(1){
		memset(buffer,0,sizeof(buffer));
		//wait user cin text
		printf("Client: \t");
		scanf("%s", &buffer[0]);
		//send the message to the server
		send(clientSocket, buffer, strlen(buffer), 0);

		if(strcmp(buffer, "disconnect") == 0){
			close(clientSocket);
			printf("[-]Disconnected from server.\n");
			exit(1);
		}
 		//Check if receiving data , and also read the incoming message
		if(recv(clientSocket, buffer, 1024, 0) < 0){
			printf("[-]Error in receiving data.\n");
		}else{
			printf("Server: \t%s\n", buffer);
		}
	}

	return 0;
}