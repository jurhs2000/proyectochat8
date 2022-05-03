#include <stdio.h>
#include <string.h>
#include <iostream>
#include "Message.h"

using namespace std;

Message::Message()
{
    message = "";
    emitter = "";
    //receptor = "";
    time = "";
}

void Message::setMessage(string message)
{
    message = message;
}

string Message::getMessage()
{
    return message;
}

void Message::setEmitter(string emmitter)
{
    emitter = emitter;
}

string Message::getEmitter()
{
    return emitter;
}

/*void Message::setReceptor(string receptor)
{
    receptor = receptor;
}*/

/*string Message::getReceptor()
{
    return receptor;
}*/

void Message::setTime(string time)
{
    time = time;
}

string Message::getTIme()
{
    return time;
}

    