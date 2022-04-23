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

int main(int argc, char *argv[])
{
    int sockfd;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("ERROR opening socket");
        return -1;
    }
    // Connecting to client
    struct sockaddr_in client_addr;
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(PORT);
    client_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    if (connect(sockfd, (struct sockaddr *)&client_addr, sizeof(client_addr)) < 0)
    {
        printf("\n Error! \n");
        return 1;
    }
    // Receiving message
    char buffer[1024] = {0};
    int valread = read(sockfd, buffer, 1024);
    printf("\n Message from client: %s \n", buffer);
    // Sending message
    printf("\n Enter message: ");
    scanf("%[^\n]", buffer);
    send(sockfd, buffer, strlen(buffer), 0);
    return 0;
}
