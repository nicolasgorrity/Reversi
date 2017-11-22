#include "message.h"

#define synchroValue 0x55

char *createMessage(MessageType messageType, MessageDataSend *data) {
    char *messageString = NULL;
    int length = 0;

    char type;
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

    case NEXT_TURN:
        if (data == NULL || data->board == NULL) {
            perror("game-master : message.c : createMessage() :\nError: Received NULL DataToSend structure for a NEW_TURN message\n");
            return NULL;
        }
        int nbCells = data->board->dimensions->x * data->board->dimensions->y;
        int nbBytes = nbCells/4 + (nbCells%4 > 0 ? 1 : 0);
        length = nbBytes + 4;
        type = 0x05;
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

    case END:
        length = 0;
        type = 0x04;
        content = NULL;
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
    if (data != NULL) {
        perror("game-player : message.c : extractMessage() :\nError: Parameter data (of type MessageDataRead*) must be initialized to NULL.\n");
        return (MessageType)-1;
    }

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
        checkSum += content[i];
    }
    checkSum = checkSum & 0xff;
    //Compare it to the received one to verify that the message is not corrupted
    if (checkSum != content[length]) {
        perror("game-player : message.c : extractMessage() :\nError: Checksum not verified. The receive message may be corrupted.\n");
        return (MessageType)-1;
    }

    switch(type) {
    case 0x01:
        messageType = CONNECT;
        data = (MessageDataRead*)malloc(sizeof(MessageDataRead));
        int playerNameLen = content[0];
        char *playerName = (char*)malloc((playerNameLen+1)*sizeof(char));
        int n;
        for (n=0; n<playerNameLen; n++) {
            playerName[n] = content[n+1];
        }
        playerName[playerNameLen] = '\0';
        data->playerName = playerName;
        break;

    case 0x03:
        messageType = NEW_MOVE;
        data = (MessageDataRead*)malloc(sizeof(MessageDataRead));
        data->newMoveCoords = (Coords*)malloc(sizeof(Coords));
        data->newMoveCoords->x = content[0];
        data->newMoveCoords->y = content[1];
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
