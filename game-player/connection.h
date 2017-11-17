#ifndef CONNECTION_H_INCLUDED
#define CONNECTION_H_INCLUDED

#include<stdio.h> //printf
#include<string.h> //strlen
#include<sys/socket.h> //socket
#include<arpa/inet.h> //inet_addr

/*
 *
 */
int createSocket();

/*
 *
 */
int connectSocket(int socket);

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
int disconnect(int socket);

#endif // CONNECTION_H_INCLUDED
