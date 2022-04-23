/**
 * @file client.c
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
    sockdf = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("ERROR opening socket");
        return -1;
    }
    // Creating server
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    // Connecting to server
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\n Error! \n");
        return 1;
    }
    // Sending message
    char buffer[1024] = {0};
    printf("\n Enter message: ");
    scanf("%[^\n]", buffer);
    send(sockfd, buffer, strlen(buffer), 0);
    // Receiving message
    int valread = read(sockfd, buffer, 1024);
    printf("\n Message from server: %s \n", buffer);
    return 0;
}
