# proyectochat8
A project made in C using sockets to create a server and client communication in Linux

Using GTK4 installed using the following command
```sh
sudo apt-get install libgtk-4-dev
```
Using cJSON lib, install with following command
```sh
sudo apt-get install libcjson-dev
```
And to compile the code use the following command
```sh
gcc -o client client.c `pkg-config --cflags --libs gtk4` -lcjson
```
To compile the server we need to link the pthread library using the next command
```sh
g++ -o server server.cpp Message.cpp User.cpp -lpthread
```
