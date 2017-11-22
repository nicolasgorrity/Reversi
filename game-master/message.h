#ifndef MESSAGE_H_INCLUDED
#define MESSAGE_H_INCLUDED

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "datastruct.h"

char *createMessage(MessageType message, MessageDataSend *data);
MessageType extractMessage(char *message, MessageDataRead *data);
char colorToChar(Color color);

#endif // MESSAGE_H_INCLUDED
