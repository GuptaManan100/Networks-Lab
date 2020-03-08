#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <signal.h> 
#include <pthread.h>
#include "server.h"
#include "lock.h"
#include "transaction.c"

//Global Variables
pthread_t tid; //Unique Thread_ID
int serverSocket, connectionSocket; //Server and Connection Sockets
struct sockaddr_in serverAddress; //Server and socket address
int opt = 1; 
int addressLen = sizeof(serverAddress); //Length of address

//Function to send an error packet to the client
void sendProblem(int sock)
{
	char * x = "1 Protocol Error";
	write(sock,x,sizeof(x));
}

//Function that runs for each client thread
void* serverThread(void *csocket)
{
	//serverCode
	int total = 0;   //Total cost of the items bought by the client
	int socket = * (int *)csocket;
	char buffer[BUFFER_SIZE] = {0}; //Buffers for receiving and sending info b/w client and server
	char toSend[BUFFER_SIZE] = {0};
	int amountRead; 

	int type,upc_code,count;
	//Keep the connection alive till the client closes it
	while(1)
	{
		//Read a request from the client
		bzero(buffer, sizeof(buffer)); 
		amountRead = recv(socket,buffer,sizeof(buffer),0);
		//If the amount Read is less than 7 then it is an error
		if(amountRead<7)
		{
			sendProblem(socket);
			continue;
		}
		//Find the type, UPC Code and count from the request
		sscanf(buffer,"%d %d %d",&type,&upc_code,&count);
		if(type == 1)
		{
			//If type == 1 then we should close the connection
			sprintf(toSend,"0 %d",total); //Send tht total cost
			send(socket,toSend,sizeof(toSend),0); 
			break; //Exit the infinite loop
		}
		else if(type == 0)
		{
			//Find the product using the database for the given upc_code
			struct product prd = do_transaction(upc_code);
			if(prd.is_error==1) // The product does not exist
			{
				sprintf(toSend,"1 UPC is not found in database");
				send(socket,toSend,sizeof(toSend),0);
				continue;
			}
			//Update the total
			total+= prd.price * count;
			sprintf(toSend,"0 %d %s",prd.price,prd.name); //Send the product price and name as response
			send(socket,toSend,sizeof(toSend),0);
		}
		else
		{
			sendProblem(socket);
		}
	}
	//Close the socket
	printf("Closing Socket %d\n",socket);
	close(socket);
	return NULL;
}

// Signal Handler for Interrupt
void sigintHandler(int sig_num) 
{ 
    signal(SIGINT, sigintHandler);
    //Close the server socket before ending the program. 
    close(serverSocket);
    printf("\nExited Gracefully\n");
    exit(0);
} 

int main(int argc, char const *argv[]) 
{
	signal(SIGINT, sigintHandler);  //Assign siginHandler for interrupt handling

	//If there are not two arguements then print an error and exit
	if (argc != 2 || atoi(argv[1])<=0 )
	{
		printf("Usage:- ./server <port number> \n");
		exit(FAILURE);
	}

	//Create a server socket and in case of error exit
	if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) <= 0) 
    { 
        printf("Socket Creation Failed\n"); 
        exit(FAILURE); 
    } 

    //Set socket options, in case of error exit
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) 
    { 
        printf("Socket Already in Use\n");
        exit(EXIT_FAILURE); 
    } 


    //Set the server addresss family to IPv4 and the port to the input argument
    serverAddress.sin_family = AF_INET; 
    serverAddress.sin_addr.s_addr = INADDR_ANY; 
    serverAddress.sin_port = htons( atoi(argv[1]) ); 

    //Bind the socket to the given address, exit in case of error
    if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress))<0) 
    { 
        printf("Server Socket Bind Failed\n"); 
        exit(FAILURE); 
    } 

    //Start listening on the socket, exit in case of error
    if (listen(serverSocket, BACKLOG) < 0) 
    { 
        printf("Error in Listen\n"); 
        exit(FAILURE); 
    } 

    //Infinite Loop for listening
    while(1)
    {
    	//printf("%s\n","Waiting for new connection" );
    	//Wait for new connection and accept it on its arrival, otherwise print error.
    	connectionSocket = accept(serverSocket, (struct sockaddr *)&serverAddress, (socklen_t*)(&addressLen));
    	if(connectionSocket<0)
    	{
    		printf("Connection Error\n");
    		fflush(stdout);
    	}
    	//Start a new thread for the client otherwise print error
    	if(pthread_create(&tid, NULL, &serverThread, &connectionSocket))
    	{
    		printf("Error creating thread\n");
    		fflush(stdout);
    	}
    }
}