#include "message.h"

#define synchroValue 0x55

char *createMessage(MessageType messageType, MessageDataSend *data) {
    char *messageString = NULL;
    int length = 0;

    char type;
    char *content;
    char checksum = 0;

    switch (messageType) {
    case NOK:
        length = 1;
        type = 0x02;
        //Allocate char array to store the message content
        content = (char*)malloc(length*sizeof(char));
        //Fill the data
        content[0] = 0x00; //not ok code
        break;

    case NEW_MOVE:
        if (data == NULL || data->coords == NULL) {
            perror("game-player : message.c : createMessage() :\nError: Received NULL DataToSend structure for a NEW_MOVE message\n");
            return NULL;
        }
        length = 2;
        type = 0x03;
        //Allocate char array to store the message content
        content = (char*)malloc(length*sizeof(char));
        //Fill the data
        content[0] = data->coords->x; //x new position
        content[1] = data->coords->y; //y new position
        break;

    default:
        return NULL;
    }

    //Calculate the checksum
    int i;
    for (i=0; i<length; i++) {
        checksum += content[i];
    }
    checksum = (type+checksum) & 0xff;

    //Allocate final message string
    messageString = (char*)malloc((length+4)*sizeof(char));

    //Fill the final message string
    messageString[0] = synchroValue;
    messageString[1] = (char)length;
    messageString[2] = type;
    for (i=0; i<length; i++) {
        messageString[i+3] = content[i];
    }
    messageString[length+3] = checksum;

    //Free the remote 'content' string
    free(content);

    return messageString;
}

MessageType extractMessage(char *message, MessageDataRead *data) {
    MessageType messageType;

    //Check synchronization value
    if (message[0] != synchroValue) return (MessageType)-1;

    //Get length of the message
    int length = (int)message[1];

    //Get the type of the message
    char type = message[2];

    //Point to the message content
    char *content = message + 3;

    //Calculate the checksum
    char checkSum = type;
    int i;
    for (i=0; i<length; i++) {
        checkSum += content[i+3];
    }
    checkSum = checkSum & 0xff;
    //Compare it to the received one to verify that the message is not corrupted
    if (checkSum != message[length+3]) {
        perror("game-player : message.c : extractMessage() :\nError: Checksum not verified. The receive message may be corrupted.\n");
        return (MessageType)-1;
    }

    switch(type) {
    case 0x10:
        messageType = INIT_OK;
        if (content[0] == 0x01) data->color = BLACK;
        else if (content[0] == 0x02) data->color = WHITE;
        else perror("game-player : message.c : extractMessage() : \nError: Received unknown player color\n");
        break;

    case 0x02:
        if (content[0] == 0x01) messageType = OK;
        else if (content[0] == 0x00) messageType = NOK;
        else perror("game-player : message.c : extractMessage() : \nError: Received bad OK/NOK message\n");
        break;

    case 0x05:
        messageType = NEXT_TURN;
        //Create or update data?
    }
}
