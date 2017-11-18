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
        data = (MessageDataRead*)malloc(sizeof(MessageDataRead));
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
        data = (MessageDataRead*)malloc(sizeof(MessageDataRead));
        //Received a board : allocate the data structure
        data->board = (Board*)malloc(sizeof(Board));
        data->board->lastMove = (Coords*)malloc(sizeof(Coords));
        data->board->dimensions = (Coords*)malloc(sizeof(Coords));
        //Fill the attributes which concern the board parameters
        data->board->lastMove->x = content[0];
        data->board->lastMove->y = content[1];
        data->board->dimensions->x = content[2];
        data->board->dimensions->y = content[3];
        //Allocate the board state according to the board dimensions
        data->board->state = (char**)malloc(data->board->dimensions->y*sizeof(char*));
        for (i=0; i<data->board->dimensions->y; i++) {
            data->board->state[i] = (char*)malloc(data->board->dimensions->x*sizeof(char));
        }
        //Fill the board state
        char *messageBoardState = content + 4;
        int dataChar, board_i=0, board_j=0;
        char exit=0;
        for (dataChar=0; dataChar<length; dataChar++) {
            char factor = 0b11000000;
            int r, byteDivider = 6; //6: 2 most significants bits // 4: 2 bits after //2: 2 bits after  // 0: 2 least significant bits
            for (r=0; r<4; r++) { //4 cells are encoded on one byte
                //Only retrieve the 2 interesting bits: isolate them with factor ; translate them as LSBs with byteDivider
                data->board->state[board_i][board_j] = (messageBoardState[dataChar]&factor)>>byteDivider;
                //Prepare the next cell to be filled
                board_j++;
                //If we got to the extremity of the row, go back to the beginning of next row
                if (board_j == data->board->dimensions->x) {
                    board_j = 0;
                    board_i++;
                }
                //If we got to the end of the board, exit this loop
                if (board_i == data->board->dimensions->y) {
                    exit = 1;
                    break;
                }
                //Update the bits manipulators
                factor >> 2; //We want the next two bits of the bytes
                byteDivider -= 2; //They are two bits closer to the LSBs
            }
            if (exit) break;
        }
        //Check the dimensions
        if (board_i != data->board->dimensions->y || board_j != 0 || dataChar == length) { //The loop should always be interrupted by the exit variable
            perror("game-player : message.c : extractMessage() : \nError: Board state does not match with specified dimensions, or was not entirely retrieved\n");
        }
        break;
    }
}
