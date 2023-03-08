#include <stdio.h>
#include <string.h>   //strlen
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>   //close
#include <arpa/inet.h>    //close
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros

#define TRUE   1
#define FALSE  0
#define PORT 4444

int main()
{   int noClientsConnected = 0;
    int masterSocket , addrlen , newSocket , clientSocket[5] ,
          maxClientsCount = 5 , activity, i , valread , SD;
    int maxSD;
    struct sockaddr_in address;

    char buffer[1024];  //data buffer 

    //set of socket descriptors
    fd_set readfds;

    printf("FD_SETSIZE %d\n",FD_SETSIZE);

    //initialise all clientSocket[] to 0 so not checked
    for (i = 0; i < maxClientsCount; i++)
    {
        clientSocket[i] = 0;
    }

    //create a master socket
    if( (masterSocket = socket(AF_INET , SOCK_STREAM , 0)) == 0)
    {
        perror("[-]socket failed");
        exit(EXIT_FAILURE);
    }

    //type of socket created
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );

    //bind the socket to the port 4444
    if (bind(masterSocket, (struct sockaddr *)&address, sizeof(address))<0)
    {
        perror("[-]bind failed");
        exit(EXIT_FAILURE);
    }
    printf("[+]Listener on port %d \n", PORT);

    //try to specify maximum of 5 pending connections for the master socket
    if (listen(masterSocket, 5) < 0)
    {
        perror("[-]listen");
        exit(EXIT_FAILURE);
    }

    //accept the incoming connection
    addrlen = sizeof(address);
    puts("[+]Waiting for connections ...");

    while(TRUE)
    {
	//clear buffer
	memset(buffer, 0, sizeof(buffer));

        //clear the socket set
        FD_ZERO(&readfds);

        //add master socket to set
        FD_SET(masterSocket, &readfds);
        maxSD = masterSocket;

        //add child sockets to set
        for ( i = 0 ; i < maxClientsCount ; i++)
        {
            //socket descriptor
            SD = clientSocket[i];

            //if valid socket descriptor then add to read list
            if(SD > 0)
                FD_SET( SD , &readfds);

            //highest file descriptor number, need it for the select function
            if(SD > maxSD)
                maxSD = SD;
        }

        //wait for an activity on one of the sockets , timeout is NULL ,
        //so wait indefinitely
        activity = select( maxSD + 1 , &readfds , NULL , NULL , NULL);

        if ((activity < 0) && (errno!=EINTR))
        {
            printf("[-]select error");
        }

        //If something happened on the master socket ,
        //then its an incoming connection
        if (FD_ISSET(masterSocket, &readfds))
        {
            if ((newSocket = accept(masterSocket,
                    (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
            {
                perror("[-]accept");
                exit(EXIT_FAILURE);
            }

            //inform user of socket number - used in send and receive commands
            printf("[+]New connection , socket fd is %d\n", newSocket);

            if(noClientsConnected >= maxClientsCount)
            {	
		send(newSocket, "true" , 5,0);
                close(newSocket);
                printf( "[-]can't connect because too many clients connected \n");
            }
            else
            {
		send(newSocket , "false" , 5,0);
                noClientsConnected++;

                //add new socket to array of sockets
                for (i = 0; i < maxClientsCount; i++)
                {
                    //if position is empty
                    if( clientSocket[i] == 0 )
                    {
                        clientSocket[i] = newSocket;
                        //printf("Adding to list of sockets as %d\n" , i+1);

                        break;
                    }
                }
            }
            printf("number of clients connected is 1: %d\n",noClientsConnected);
        }
        //else its some IO operation on some other socket
        for (i = 0; i < maxClientsCount; i++)
        {
            SD = clientSocket[i];

            if (FD_ISSET( SD , &readfds))
            {
                //Check if it was for closing , and also read the
                //incoming message
                if ((valread = read( SD , buffer, 1024)) == 0)
                {
                    //Somebody disconnected , get his details and print
                    getpeername(SD , (struct sockaddr*)&address , \
                        (socklen_t*)&addrlen);
                    printf("[-]Host disconnected ");
			
                    noClientsConnected--;
                    printf("number of clients connected is 2: %d\n", noClientsConnected);
                    //Close the socket and mark as 0 in list for reuse
                    close( SD );
                    clientSocket[i] = 0;}
                //send back the message that came in
                else
                {
		char* command = getenv(buffer);
		if (command==NULL)
		{ 
			send(SD, "invalid command", 15, 0);
		}
		else{
		send(SD , command , strlen(command),0);}
		}
	     }
	   }
	}
return 0;
}