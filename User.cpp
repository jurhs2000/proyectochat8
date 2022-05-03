#include <stdio.h>
#include <string.h>
#include <iostream>
#include <time.h>
#include "user.h"

using namespace std;

User::User()
{
    userName = "";
    status = "0";
    socketId = 0;
    lastConnection = 0;
}

void User:: setUsername(string userName)
{
    userName = userName;
}

string User:: getUsername()
{
    return userName;
}

void User::setSocket(int socketId)
{
    socketId = socketId;
}

int User::getSocket()
{
    return socketId;
}

void User::setLastConnection()
{
    time(&lastConnection);
}

time_t User::getLastConnection()
{
    return lastConnection;
}

