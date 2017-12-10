#include "message.h"

#define synchroValue 0x55

String *createMessage(MessageType messageType, MessageDataSend *data) {
    char *messageString = NULL;
    unsigned short length = 0;

    char type = 0x00;
    char *content;
    char checksum = 0;

    switch (messageType) {
    case INIT_OK:
        if (data == NULL) {
            perror("game-master : message.c : createMessage() :\nError: Received NULL DataToSend structure for a INIT_OK message\n");
            return NULL;
        }
        length = 1;
        type = 0x10;
        //Allocate char array to store the message content
        content = (char*)malloc(length*sizeof(char));
        //Fill the data
        content[0] = colorToChar(data->playerColor);
        break;

    case OK:
        length = 1;
        type = 0x02;
        //Allocate char array to store the message content
        content = (char*)malloc(length*sizeof(char));
        //Fill the data
        content[0] = 0x01;
        break;

    case NOK:
        length = 1;
        type = 0x02;
        //Allocate char array to store the message content
        content = (char*)malloc(length*sizeof(char));
        //Fill the data
        content[0] = 0x00;
        break;

    case STATUS1:
        type = 0x06;
    case NEXT_TURN:
        if (data == NULL || data->board == NULL) {
            perror("game-master : message.c : createMessage() :\nError: Received NULL DataToSend structure for a NEXT_TURN or STATUS1 message\n");
            return NULL;
        }
        int nbCells = data->board->dimensions->x * data->board->dimensions->y;
        int nbBytes = nbCells/4 + (nbCells%4 > 0 ? 1 : 0);
        length = nbBytes + 4;
        if (type == 0x00) type = 0x05;
        //Allocate char array to store the message content
        content = (char*)malloc(length*sizeof(char));
        //Fill the data
        content[0] = data->board->lastMove->x;
        content[1] = data->board->lastMove->y;
        content[2] = data->board->dimensions->x;
        content[3] = data->board->dimensions->y;
        char *boardState = content + 4;
        int i, j, board_i=0, board_j=0;
        for (i=0; i<nbBytes; i++) {
            char bitsDuos[4];
            char end = 0;
            for (j=0; j<4; j++) {
                if (end == 1) bitsDuos[j] = 0x00;
                else {
                    bitsDuos[j] = colorToChar(data->board->state[board_i][board_j]);
                    board_j++;
                    if (board_j == data->board->dimensions->x) {
                        board_j = 0;
                        board_i++;
                    }
                    if (board_i == data->board->dimensions->y) {
                        end = 1;
                        if (i!=nbBytes-1) perror("game-master : message.c : createMessage() : Strange behaviour: reaches end of board before the last byte\n");
                    }
                }
            }
            char byte = 0x00;
            int byteDivider = 6;
            for (j=0; j<4; j++) {
                byte = byte | (bitsDuos[j] << byteDivider);
                byteDivider -= 2;
            }
            boardState[i] = byte;
        }
        break;

    case STATUS2:
        if (data == NULL) {
            perror("game-master : message.c : createMessage() :\nError: Received NULL DataToSend structure for a STATUS2 message\n");
            return NULL;
        }
        unsigned short whiteNameLen = data->playersData->dataWP->playerName->length;
        unsigned short blackNameLen = data->playersData->dataBP->playerName->length;
        length = whiteNameLen + blackNameLen + 8;
        type = 0x07;
        //Allocate char array to store the message content
        content = (char*)malloc(length*sizeof(char));
        //Fill the data
        content[0] = data->playersData->dataBP->points; //black score
        content[1] = data->playersData->dataBP->timer >> 8; //black timer MSBs
        content[2] = data->playersData->dataBP->timer; //black timer LSBs
        content[3] = blackNameLen;
        unsigned short r;
        for (r=0; r<blackNameLen; r++) {
            content[4+r] = data->playersData->dataBP->playerName->text[r];
        }
        char *content2 = content + 4 + blackNameLen;
        content2[0] = data->playersData->dataWP->points; //black score
        content2[1] = data->playersData->dataWP->timer >> 8; //black timer MSBs
        content2[2] = data->playersData->dataWP->timer; //black timer LSBs
        content2[3] = whiteNameLen;
        for (r=0; r<whiteNameLen; r++) {
            content2[4+r] = data->playersData->dataWP->playerName->text[r];
        }
        break;

    case END:
        length = 0;
        type = 0x04;
        content = NULL;
        break;

    case PING:
        length = 0;
        type = 0x11;
        content = NULL;

    default:
        return NULL;
    }

    //Calculate the checksum
    int i;
    for (i=0; i<length; i++) {
        checksum += (unsigned char)content[i];
    }
    checksum = (type+checksum) & 0xff;

    //Allocate final message string
    messageString = (char*)malloc((length+5)*sizeof(char));

    //Fill the final message string
    messageString[0] = synchroValue;
    messageString[1] = length;
    messageString[2] = type;
    for (i=0; i<length; i++) {
        messageString[i+3] = content[i];
    }
    messageString[length+3] = checksum;
    messageString[length+4] = '\0';

    //Free the remote 'content' string
    if (length > 0) free(content);

    String *string = newString(messageString, length+4);
    return string;
}

MessageType extractMessage(char *message, MessageDataRead *data) {
    MessageType messageType;

    //Check synchronization value
    if (message[0] != synchroValue) return (MessageType)-1;

    //Get length of the message
    int length = message[1];

    //Get the type of the message
    char type = message[2];

    //Point to the message content
    char *content = message + 3;

    //Calculate the checksum
    unsigned char checkSum = 0;
    int i;
    for (i=0; i<length; i++) {
        checkSum += (unsigned char)content[i];
    }
    checkSum = ((unsigned char)type+checkSum) & 0xff;

    //Compare it to the received one to verify that the message is not corrupted
    if (checkSum != (unsigned char)content[length]) {
        perror("game-player : message.c : extractMessage() :\nError: Checksum not verified. The receive message may be corrupted.\n");
        printf("Read checksum is %x\n", (unsigned char)checkSum);
        printf("Expected checksum was %x\n", (unsigned char)content[length]);
        return (MessageType)-1;
    }

    switch(type) {
    case 0x01:
        messageType = CONNECT;
        int playerNameLen = (int)content[0];
        char *playerName = (char*)malloc((playerNameLen+1)*sizeof(char));
        int n;
        for (n=0; n<playerNameLen; n++) {
            playerName[n] = content[n+1];
        }
        playerName[playerNameLen] = '\0';
        data->playerName = newString(playerName, playerNameLen);
        break;

    case 0x02:
        if (content[0] == 0x01) messageType = OK;
        else if (content[0] == 0x00) messageType = NOK;
        break;

    case 0x03:
        messageType = NEW_MOVE;
        data->newMoveCoords = (Coords*)malloc(sizeof(Coords));
        data->newMoveCoords->x = (unsigned short)content[0];
        data->newMoveCoords->y = (unsigned short)content[1];
        break;

    case 0x08:
        messageType = CONTROL;
        data->control = (ControlData*)malloc(sizeof(ControlData));
        data->control->newBoardSize = (Coords*)malloc(sizeof(Coords));
        data->control->newBoardSize->x = content[0];
        data->control->newBoardSize->y = content[1];
        data->control->mode = content[2];
        data->control->restart = content[3];
        break;

    default:
        messageType = (MessageType)-1;
    }
    return messageType;
}

char colorToChar(Color color) {
    char toReturn;
    switch (color) {
        case EMPTY:
            toReturn = 0b00000000;
            break;
        case BLACK:
            toReturn = 0b00000001;
            break;
        case WHITE:
            toReturn = 0b00000010;
            break;
        default:
            perror("game-master : message.c : colorToChar() : \nError: Received incompatible color value\n");
            toReturn = (char)-1;
    }
    return toReturn;
}
