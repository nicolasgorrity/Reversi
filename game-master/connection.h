#ifndef CONNECTION_H_INCLUDED
#define CONNECTION_H_INCLUDED

#include<stdio.h> //printf
#include<string.h> //strlen
#include<sys/socket.h> //socket
#include<unistd.h>//socket read write
#include<arpa/inet.h> //inet_addr
#include<stdlib.h>//malloc


/*
 *
 */
int createSocket();

/*
 *
 */
int bindSocket(int socket, unsigned int port);

/*
 *
 */
int listenSocket(int socket);

/*
 *
 */
int acceptSocket(int socket);

/*
 *
 */
char* readMessage(int socket);

/*
 *
 */
int writeMessage(int socket, char *message);

/*
 *
 */
int disconnect(int socket, int sock1, int sock2, int sock3);


#endif // CONNECTION_H_INCLUDED
