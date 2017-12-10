#ifndef MESSAGE_H_INCLUDED
#define MESSAGE_H_INCLUDED

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "datastruct.h"

String *createMessage(MessageType message, MessageDataSend *data);
MessageType extractMessage(char *message, MessageDataRead *data);
Color charToCellColor(unsigned char cellState);

#endif // MESSAGE_H_INCLUDED
