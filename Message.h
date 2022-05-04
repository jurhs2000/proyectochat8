#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class Message
{
    public:
        string message;
        string emitter;
        string receptor;
        string time;
        Message();
        void setMessage(string message);
        string getMessage();
        void setEmitter(string emitter);
        string getEmitter();
        void setReceptor(string receptor);
        string getReceptor();
        void setTime(string time);
        string getTIme();
};
