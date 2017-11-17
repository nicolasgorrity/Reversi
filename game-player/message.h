#ifndef MESSAGE_H_INCLUDED
#define MESSAGE_H_INCLUDED

typedef enum {OK, NOK, CONNECT, NEXT_TURN, NEW_MOVE} Message;

char *createMessage(Message message);

#endif // MESSAGE_H_INCLUDED
