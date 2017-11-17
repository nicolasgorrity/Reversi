#include "connection.h"

int createSocket()
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        printf("game-player : connection.c in function createSocket() : Could not create socket \n");
    }
    puts("Socket created \n");
    return sock;
}

int connectSocket(int socket)
{
    struct sockaddr_in server;
    //Define the socket remote address : 127.0.0.1:8888
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons(8888);

    //Connect to remote server
    if (connect(socket, (struct sockaddr*)&server, sizeof(server)) < 0)
    {
        perror("game-player : connection.c in function connectSocket() : Connect failed. Error \n");
        return -1;
    }
    puts("Connected \n");
    return 0;
}

char* readMessage(int socket)
{
    char message[2000];
    if (read(socket, message, 2000) < 0)
    {
        puts("game-player : connection.c in function readMessage() : read failed \n");
        return NULL;
    }

    int msgLength = message[1];
    char *shortMessage;
    shortMessage = (char*) malloc((msgLength+4)*sizeof(char));
    int i;
    for(i=0; i<(msgLength+4); i++)
    {
        shortMessage[i]=message[i];
    }
    return message;
}


int writeMessage(int socket, char *message)
{
    if (write(socket, message, strlen(message), 0) < 0)
    {
        puts("game-player : connection.c in function writeMessage() : write failed");
        return -1;
    }
    return 0;
}


int disconnect(int socket)
{
    close(socket);
}
