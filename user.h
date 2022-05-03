#include <iostream>
#include <string>
#include <time.h>

using namespace std;

class User {
    public:
        string userName;
        string status;
        time_t lastConnection;
        int socketId;
        User();
        void setUsername(string userName);
        string getUsername();
        void setSocket(int socketId);
        int getSocket();
        void setLastConnection();
        time_t getLastConnection();
};
