#include "connection.h"

int createSocket()
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        printf("game-master : connection.c in function createSocket() : Could not create socket \n");
    }
    puts("Socket created\n");
    return sock;
}

int bindSocket(int socket, unsigned int port)
{
    struct sockaddr_in server;
    //Define the socket remote address : 127.0.0.1:8888
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons(port);

    //Connect to remote server
    if (bind(socket, (struct sockaddr*)&server, sizeof(server)) < 0)
    {
        perror("game-master : connection.c in function bindSocket() : Bind failed. Error \n");
        return -1;
    }
    printf("Bind port %d\n", port);
    return 0;
}

int listenSocket(int socket)
{
    puts("Waiting for incoming connections...\n");
    listen(socket, 3);
    return 0;
}

int acceptSocket(int socket)
{
    int client_sock;
    struct sockaddr_in client;
    int c = sizeof(struct sockaddr_in);
    client_sock = accept(socket, (struct sockaddr*)&client, (socklen_t*)&c);
    if (client_sock < 0)
    {
        perror("game-master : connection.c in function acceptSocket() : accept failed\n");
        return -1;
    }
    printf("Connection accepted\n");
    return client_sock;
}

char* readMessage(int socket)
{
    char message[2000];
    if (read(socket, message, 2000) < 0)
    {
        puts("game-master : connection.c in function readMessage() : read failed \n");
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
    return shortMessage;
}

int writeMessage(int socket, String *message)
{
    if (write(socket, message->text, message->length) < 0)
    {
        printf("game-master : connection.c in function writeMessage() : write failed.\n");
        return -1;
    }
    return 0;
}

int disconnect(int socket1, int sock1, int sock2, int sock3)
{
    printf("game-master : End of the Game. Disconnecting...\n");
    char buf[1];
    if (sock1 >= 0) {
        while (recv(sock1, buf, 1, 0) > 0); //wait for client to close sock1
        int res = close(sock1);
        if (res < 0) perror("game-master : connection.c : could not disconnect client socket 1\n");
    }
    if (sock2 >= 0) {
        while (recv(sock2, buf, 1, 0) > 0); //wait for client to close sock2
        int res = close(sock2);
        if (res < 0) perror("game-master : connection.c : could not disconnect client socket 2\n");
    }
    if (sock3 >= 0) {
        while (recv(sock3, buf, 1, 0) > 0); //wait for client to close sock3
        int res = close(sock3);
        if (res < 0) perror("game-master : connection.c : could not disconnect client socket 3\n");
    }
    int res = close(socket1);
    if (res < 0) perror("game-master : connection.c : could not disconnect server socket1\n");
    printf("game-master stopping...\n");
    return 0;
}
