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
const string getUsers();
void removeUser();
void setStatus(string username);
using namespace std;
using json = nlohmann::json;
vector<Message> messages_list;
map<string, User *> user_list = {};
// std::list<Message> messages_list;

int main(int argc, char *argv[])
{
	// int port;
	// port = atoi(argv[1]);
	// User user1;
	// User user2;
	// user1.userName = "oscarsaravia";
	// user1.status = "1";
	// user1.socketId = 1;
	// user1.lastConnection = 10;
	// user_list["oscarsaravia"] = &user1;

	// user2.userName = "jurhs2";
	// user2.status = "1";
	// user2.socketId = 1;
	// user2.lastConnection = 10;
	// user_list["oscarsaravia"] = &user2;

	// Message mensaje1;
	// Message mensaje2;
	// mensaje1.message = "HOLA: ESTE ES EL MENSAJE 1";
	// mensaje1.emitter = "RAHUL";
	// mensaje1.receptor = "Juhrs";
	// mensaje1.time = "15:59";

	// mensaje2.message = "HOLA: ESTE ES EL MENSAJE 2";
	// mensaje2.emitter = "RAHUL";
	// mensaje2.receptor = "JUhrs";
	// mensaje2.time = "15:59";

	// messages_list.push_back(mensaje1);
	// messages_list.push_back(mensaje2);

	std::string list_of_messages = getMessages(messages_list);
	// printf("%s", list_of_messages.c_str());

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
    server.sin_port = htons(8888);
    // server.sin_addr.s_addr = inet_addr("172.31.38.117");
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
	// if (new_socket<0)
	// {
	// 	perror("accept failed");
	// 	return 1;
	// }
    return 0;
}

void addUser(){
	
}

void removeUser(){

}

void addMessage(Message message){
	messages_list.push_back(message);
}

const string getUsers(string type)
{
	if (type == "all")
	{
		json data;
		json *data_ptr;
		data["response"] = "GET_USER";
		data["body"] = json::array();
		for(const auto &user: user_list)
		{
			json userToAdd = json::array({user.first, user.second->status});
			data["body"].push_back(userToAdd);
		}
		std::string s = data.dump();
		return s;
		// cout << data.dump().c_str() <<  endl;
	}
	else {
		json data;
		json *data_ptr;
		data["response"] = "GET_USER";
		User *user2 = user_list[type];
		string tempIp = "1.2.3.4";
		string tempStatus = user2->status;
		data["body"] = json::array();
		data["body"].push_back(tempIp);
		data["body"].push_back(tempStatus);
		std::string s = data.dump();
		return s;
		// cout << data.dump().c_str() <<  endl;
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
		json list_of_messages = json::array({item.message, item.emitter, item.time});
		data["body"].push_back(list_of_messages);
	}
	//Reply to the client
	std::string s = data.dump();
	return s;
}

void setStatus(string username)
{
	// json data;
	// data["response"] = "PUT_STATUS";
	// data["code"] = 200;
	// auto it = user_list.find(currentUser.userName);
	// if (it == user_list.end()) {
	// 	cout << " NOT FOUND " << endl;
	// } else {
	// 	currentUser.status = j[]
	// }
	// // User *user2 = user_list[type];
}


/*
 * This will handle connection for each client
 * */
void *connection_handler(void *socket_desc)
{
	User currentUser;
	User *new_user = (User *)socket_desc;
	int user_socket = new_user->getSocket();
	string user_n = new_user->getUsername();

	//Get the socket descriptor
	int sock = *(int*)socket_desc;
	int read_size;
	char client_message[2000];

	while (1)
	{
		try
		{
			char buffer[8192];
			memset(buffer, 0, 8192);
			int recived = recv(sock, buffer, 8192, 0);
			if (recived > 0)
			{
				printf("Recibido buffer: %s\n", buffer);
				auto j = json::parse(buffer);
                // cout << j << "json" << endl;

                string request = j["request"];
				// printf("REQUESTTTT: %s", request.c_str());
				cout << " REQUEST JSON " << request << endl;
                if (request == "INIT_CONEX")
                {
					cout << " CLIENT ASKED FOR INIT CONEX " << endl;
					// auto userMetadata = json::parse(client_reply);
					if (user_list.find(j["body"][1]) == user_list.end()) {
						// SIGNIFICA QUE EL USUARIO NO EXISTE
						currentUser.userName = j["body"][1];
						// newUser.lastConnection
						currentUser.userName = j["body"][1];
						currentUser.status = "1";
						currentUser.socketId = user_socket;
						cout << " USER ID: " << j["body"][1] << endl;
						user_list[j["body"][1]] = &currentUser;
						json data;
						data["response"] = "INIT_CONEX";
						data["code"] = 200;
						write(sock, data.dump().c_str(), strlen(data.dump().c_str()));
					} else {
						// SIGNIFICA QUE EL USUARIO YA EXISTE
						cout << " EL USUARIO YA EXISTE " << endl;
					}
				}
				else if (request == "END_CONEX")
				{

				}
				else if (j["request"] == "GET_CHAT")
				{
					cout << " CLIENT REQUESTED GET_CHAT: " << j["body"] << endl;
					std::string list_of_messages = getMessages(messages_list);
					write(sock, list_of_messages.c_str(), strlen(list_of_messages.c_str()));
					cout << " SERVER RESPONDED" << list_of_messages.c_str() << endl;
				}
				else if (request == "POST_CHAT")
				{
					cout << " CLIENT REQUESTED POST_CHAT FROM: " << j["body"][1] << " TO: " << j["body"][3] << endl;
					Message tempMessage;
					tempMessage.message = j["body"][0];
					tempMessage.emitter = j["body"][1];
					tempMessage.time = j["body"][2];
					tempMessage.receptor = j["body"][3];
					addMessage(tempMessage);
					json resp;
					resp["response"] = "POST_CHAT";
					resp["code"] = 200;
					write(sock, resp.dump().c_str(), strlen(resp.dump().c_str()));
					cout << " SERVER RESPONDED" << resp.dump().c_str() << endl;

					if (j["body"][3] == "all")
					{
						Message socketMessage;
						map<string, User *>::iterator it;
						json response = json::object({{"response", "NEW_MESSAGE"}});
						for (it = user_list.begin(); it != user_list.end(); it++)
						{
							User *user = it->second;
							if (user->userName != j["body"][1])
							{
								response["body"] = json::array({j["body"][0], j["body"][1], j["body"][2], j["body"][3]});
								write(user->socketId, response.dump().c_str(), strlen(response.dump().c_str()));
							}
						}
						cout << " SOCKET MESSAGE SENT " << response << endl;
					}
					else {
						json response = json::object({{"response", "NEW_MESSAGE"}});

						map<string, User *>::iterator it;
						for (it = user_list.begin(); it != user_list.end(); it++)
						{
							User *user = it->second;
							if (user->userName != j["body"][3])
							{
								response["body"] = json::array({j["body"][0], j["body"][1], j["body"][2], j["body"][3]});
								cout << " USUARIO ENCONTRADO " << user->userName << user->socketId << endl;
								write(user->socketId, response.dump().c_str(), strlen(response.dump().c_str()));
							}
						}
						// write(socketNUm, response.dump().c_str(), strlen(response.dump().c_str()));
					}
				}
				else if (j["request"] == "GET_USER")
				{
					cout << " CLIENT REQUESTED GET_USER FROM: " << j["body"] << endl;
					if (j["body"] == "all")
					{

					}
					string userRequested = j["body"];
					std::string list_of_users = getUsers(userRequested);
					// send(user_socket, list_of_users.c_str(), strlen(list_of_users.c_str()), 0);
					write(sock, list_of_users.c_str(), strlen(list_of_users.c_str()));
					cout << " SERVER RESPONDED" << list_of_users.c_str() << endl;
				}
				else if (request == "PUT_STATUS")
				{
					json data;
					data["response"] = "PUT_STATUS";
					data["code"] = 200;
					auto it = user_list.find(currentUser.userName);
					if (it == user_list.end()) {
						cout << " NOT FOUND " << endl;
					} else {
						currentUser.status = j["body"];
					}
					cout << "USER USERNAME IN PUT " << currentUser.userName << currentUser.status << endl;
					// cout << " PUT STATUS TO: " << currentUser.userName << endl;
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
