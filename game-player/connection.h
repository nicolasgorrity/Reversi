#ifndef CONNECTION_H_INCLUDED
#define CONNECTION_H_INCLUDED

/*
 *
 */
int createSocket();

/*
 *
 */
int connect(int socket);

/*
 *
 */
int readMessage(int socket);

/*
 *
 */
int writeMessage(int socket);

#endif // CONNECTION_H_INCLUDED
