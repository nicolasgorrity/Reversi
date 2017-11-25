#ifndef MESSAGE_H_INCLUDED
#define MESSAGE_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>

#include "datastruct.h"

char *createMessage(MessageType message, MessageDataSend *data);
MessageType extractMessage(char *message, MessageDataRead *data);
Color charToCellColor(char cellState);

#endif // MESSAGE_H_INCLUDED
