/**
 * @file server.c
 * @author Julio H., Oscar S.
 * @brief 
 * @version 0.1
 * 
 * Universidad del Valle de Guatemala
 * Sistemas Operativos - CC3064
 * Proyecto - Chat
 */
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

void *connection_handler(void *);

int main(int argc, char *argv[])
{
    int socket_desc, c, new_socket, *thread_socket;
    char *client_ip, client_reply[8000];
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc < 0)
    {
        perror("ERROR opening socket");
        return -1;
    }
    // Creating client
    struct sockaddr_in client, server;
    server.sin_family = AF_INET;
    server.sin_port = htons(8888);
    server.sin_addr.s_addr = INADDR_ANY;
    
    //Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        puts("bind failed");
    }
    puts("bind done");

    //Listen
	listen(socket_desc , 3);

    //Accept and incoming connection
	puts("Waiting for incoming connections...");
	c = sizeof(struct sockaddr_in);
	while( (new_socket = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
	{
		puts("Connection accepted");
        printf("%d", c);
        //Receive a reply from the server
        if( recv(new_socket, client_reply , 2000, 0) < 0)
        {
            puts("recv failed");
        }
        client_ip = inet_ntoa(client.sin_addr);
        puts("Client ip is: ");
        puts(client_ip);
        puts("Message received from client\n");
        puts(client_reply);
		
		//Reply to the client
		const char *message = "Hello Client , I have received your connection. But I have to go now, bye\n";
		write(new_socket , message , strlen(message));

        pthread_t sniffer_thread;
		thread_socket = (int*) malloc(1);
		*thread_socket = new_socket;
		
		if( pthread_create( &sniffer_thread , NULL ,  connection_handler , (void*) thread_socket) < 0)
		{
			perror("could not create thread");
			return 1;
		}
		
		//Now join the thread , so that we dont terminate before the thread
		//pthread_join( sniffer_thread , NULL);
		puts("Handler assigned");
	}
	close(socket_desc);
	if (new_socket<0)
	{
		perror("accept failed");
		return 1;
	}
    return 0;
}

/*
 * This will handle connection for each client
 * */
void *connection_handler(void *socket_desc)
{
	//Get the socket descriptor
	int sock = *(int*)socket_desc;
	int read_size;
	char client_message[2000];
	
	//Send some messages to the client
	const char *message = "Greetings! I am your connection handler\n";
	write(sock , message , strlen(message));
	
	message = "Now type something and i shall repeat what you type \n";
	write(sock , message , strlen(message));
	
	//Receive a message from client
	while( (read_size = recv(sock , client_message , 2000 , 0)) > 0 )
	{
		//Send the message back to client
		write(sock , client_message , strlen(client_message));
	}
	
	if(read_size == 0)
	{
		puts("Client disconnected");
		fflush(stdout);
	}
	else if(read_size == -1)
	{
		perror("recv failed");
	}
		
	//Free the socket pointer
	free(socket_desc);
	
	return 0;
}
