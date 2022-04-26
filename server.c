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

int main(int argc, char *argv[])
{
    int socket_desc, c, new_socket;
    char *message, client_reply[2000];
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
        if( recv(new_socket, client_reply , c, 0) < 0)
        {
            puts("recv failed");
        }
        puts("Reply received from client\n");
        puts(client_reply);
		
		//Reply to the client
		message = "Hello Client , I have received your connection. But I have to go now, bye\n";
		write(new_socket , message , strlen(message));
	}
	
	if (new_socket<0)
	{
		perror("accept failed");
		return 1;
	}

    return 0;
}
