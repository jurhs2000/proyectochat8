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
#include <map>
#include <list>
#include <iostream>
#include <vector>
#include "Message.h"
#include "user.h"
#include "json.hpp"
#include <string>


void *connection_handler(void *);
void addUser();
const string getMessages(vector<Message>);
void addMessage();
void getUsers();
void removeUser();
using namespace std;
using json = nlohmann::json;
vector<Message> messages_list;
map<string, User *> user_list = {};
// std::list<Message> messages_list;

int main(int argc, char *argv[])
{
	// int port;
	// port = atoi(argv[1]);
	User user1;
	User user2;
	user1.userName = "jurhs";
	user1.status = "1";
	user1.socketId = 1;
	user1.lastConnection = 10;
	user_list["julioherrera"] = &user1;

	user2.userName = "jurhs2";
	user2.status = "1";
	user2.socketId = 1;
	user2.lastConnection = 10;
	user_list["oscarsaravia"] = &user2;

	Message mensaje1;
	Message mensaje2;
	mensaje1.message = "HOLA: ESTE ES EL MENSAJE 1";
	mensaje1.emitter = "RAHUL";
	//mensaje1.receptor = "JUhrs";
	mensaje1.time = "15:59";

	mensaje2.message = "HOLA: ESTE ES EL MENSAJE 2";
	mensaje2.emitter = "RAHUL";
	//mensaje2.receptor = "JUhrs";
	mensaje2.time = "15:59";

	messages_list.push_back(mensaje1);
	messages_list.push_back(mensaje2);

	std::string list_of_messages = getMessages(messages_list);
	printf("%s", list_of_messages.c_str());

	const char *message = list_of_messages.c_str();
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
    server.sin_port = htons(8889);
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

void addUser(){
	
}

void removeUser(){

}

void addMessage(Message message){
	messages_list.push_back(message);
}

void getUsers(string type)
{
	if (type == "all")
	{
		json data;
		json *data_ptr;
		data["response"] = "GET_USER";
		data["body"] = json::array();
		for(const auto &user: user_list)
		{
			json userToAdd = json::array({user.second->userName, user.second->status});
			data["body"].push_back(userToAdd);
			std::cout << user.first << ' ';
		}
		printf("===============\n");
		cout << data.dump().c_str() <<  endl;
		printf("===============\n");
	}
	else {
		json data;
		json *data_ptr;
		data["response"] = "GET_USER";
		User *user2 = user_list[type];
		string burnName = user2->userName;
		data["body"] = json::array({"username", burnName});
		cout << data.dump().c_str() <<  endl;
	}
}

const string getMessages(vector<Message> messages)
{
	json data;
	json *data_ptr;
	data["response"] = "GET_CHAT";
	data["code"] = 200;
	data["body"] = json::array();
	for (auto &item: messages)
	{
		json list_of_messages = json::array({item.message, item.emitter/*, item.receptor*/, item.time});
		data["body"].push_back(list_of_messages);
	}
	//Reply to the client
	std::string s = data.dump();
	return s;
}


/*
 * This will handle connection for each client
 * */
void *connection_handler(void *socket_desc)
{
	User currentUser;
	User *new_user = (User *)socket_desc;
	int user_socket = new_user->getSocket();

	//Get the socket descriptor
	int sock = *(int*)socket_desc;
	int read_size;
	char client_message[2000];

	while (1)
	{
		try
		{
			char buffer[8192];
			int recived = recv(sock, buffer, 8192, 0);
			if (recived > 0)
			{
				printf("Recibido buffer: %s\n", buffer);
				auto j = json::parse(buffer);
                cout << j << "json" << endl;

                string request = j["request"];
				// printf("REQUESTTTT: %s", request.c_str());
				cout << " REQUEST JSON " << j << endl;

                if (request == "INIT_CONEX")
                {
					printf("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
				}
				else if (request == "END_CONEX")
				{

				}
				else if (j["request"] == "GET_CHAT")
				{

					// printf("AQUI SE HACE UN GET CHAT SIUUUUUUUUUUU");
					std::string list_of_messages = getMessages(messages_list);
					send(user_socket, list_of_messages.c_str(), strlen(list_of_messages.c_str()), 0);
					printf("%s", list_of_messages.c_str());
					// getMessages(messages_list)
				}
				else if (request == "POST_CHAT")
				{
					Message mensaje3;
					mensaje3.message = j["body"][0];
					mensaje3.emitter = j["body"][1];
					mensaje3.time = j["body"][2];
					addMessage(mensaje3);
					json resp;
					resp["response"] = "POST_CHAT";
					resp["code"] = 200;
					send(user_socket, resp.dump().c_str(), strlen(resp.dump().c_str()), 0);
				}
				else if (request == "GET_USER")
				{
					string userRequested = j["body"];
					getUsers("all");
					// getUsers(userRequested);
				}
				else if (request == "PUT_STATUS")
				{

				}
			}
		}
		catch (const std::exception &e)
        {
            printf("Hubo un error :(");
        }
	}
	
	//Send some messages to the client
	const char *message = "Greetings! I am your connection handler\n";
	write(sock , message , strlen(message));
	
	message = "Now type something and i shall repeat what you type \n";
	write(sock , message , strlen(message));
	
	//Receive a message from client
	while( (read_size = recv(sock , client_message , 2000 , 0)) > 0 )
	{
		//Send the message back to client
		puts(client_message);
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
