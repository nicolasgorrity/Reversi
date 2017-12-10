#ifndef CONNECTION_H_INCLUDED
#define CONNECTION_H_INCLUDED

#include<stdio.h> //printf
#include<string.h> //strlen
#include<sys/socket.h> //socket
#include<unistd.h>//socket read write
#include<arpa/inet.h> //inet_addr
#include<stdlib.h>//malloc

#include "datastruct.h"


/*
 *
 */
int createSocket();

/*
 *
 */
int connectSocket(int socket, unsigned int port);

/*
 *
 */
char* readMessage(int socket);

/*
 *
 */
int writeMessage(int socket, String *message);

/*
 *
 */
int disconnect(int socket);

#endif // CONNECTION_H_INCLUDED
