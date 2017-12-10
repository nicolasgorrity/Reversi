#include "message.h"

#define synchroValue 0x55

String *createMessage(MessageType messageType, MessageDataSend *data) {
    char *messageString = NULL;
    unsigned short length = 0;

    char type;
    char *content;
    char checksum = 0;

    switch (messageType) {
    case CONNECT:
        if (data == NULL || data->playerName == NULL) {
            perror("game-player : message.c : createMessage() :\nError: Received NULL DataToSend structure for a CONNECT message\n");
            return NULL;
        }
        int nameSize = strlen(data->playerName->text);
        length = nameSize + 1;
        type = 0x01;
        //Allocate char array to store the message content
        content = (char*)malloc(length*sizeof(char));
        //Fill the data
        content[0] = (char)nameSize;
        int i;
        for (i=0; i<nameSize; i++) {
            content[i+1] = data->playerName->text[i];
        }
        break;

    case NEW_MOVE:
        if (data == NULL || data->newMoveCoords == NULL) {
            perror("game-player : message.c : createMessage() :\nError: Received NULL DataToSend structure for a NEW_MOVE message\n");
            return NULL;
        }
        length = 2;
        type = 0x03;
        //Allocate char array to store the message content
        content = (char*)malloc(length*sizeof(char));
        //Fill the data
        content[0] = data->newMoveCoords->x; //x new position
        content[1] = data->newMoveCoords->y; //y new position
        break;

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
    case 0x10:
        messageType = INIT_OK;
        if (content[0] == 0x01) data->playerColor = BLACK;
        else if (content[0] == 0x02) data->playerColor = WHITE;
        else perror("game-player : message.c : extractMessage() : \nError: Received unknown player color\n");
        break;

    case 0x02:
        if (content[0] == 0x01) messageType = OK;
        else if (content[0] == 0x00) messageType = NOK;
        else perror("game-player : message.c : extractMessage() : \nError: Received bad OK/NOK message\n");
        break;

    case 0x04:
        messageType = END;
        break;

    case 0x05:
        messageType = NEXT_TURN;
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
        data->board->state = (Color**)malloc(data->board->dimensions->y*sizeof(Color*));
        for (i=0; i<data->board->dimensions->y; i++) {
            data->board->state[i] = (Color*)malloc(data->board->dimensions->x*sizeof(Color));
        }
        //Fill the board state
        char *messageBoardState = content + 4;
        int dataChar, board_i=0, board_j=0;
        char exit=0;
        for (dataChar=0; dataChar<length; dataChar++) {
            unsigned char factor = 0b11000000;
            int r, byteDivider = 6; //6: 2 most significants bits // 4: 2 bits after //2: 2 bits after  // 0: 2 least significant bits
            for (r=0; r<4; r++) { //4 cells are encoded on one byte
                //Only retrieve the 2 interesting bits: isolate them with factor ; translate them as LSBs with byteDivider
                unsigned char cellState = ((unsigned char)messageBoardState[dataChar]&factor)>>byteDivider;
                data->board->state[board_i][board_j] = charToCellColor(cellState);
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
                factor = factor >> 2; //We want the next two bits of the bytes
                byteDivider -= 2; //They are two bits closer to the LSBs
            }
            if (exit) break;
        }
        //Check the dimensions
        if (board_i != data->board->dimensions->y || board_j != 0 || dataChar == length) { //The loop should always be interrupted by the exit variable
            perror("game-player : message.c : extractMessage() : \nError: Board state does not match with specified dimensions, or was not entirely retrieved\n");
        }
        break;

    case 0x11:
        messageType = PING;
        break;

    default:
        messageType = (MessageType)-1;
    }
    return messageType;
}

Color charToCellColor(unsigned char cellState) {
    Color color;
    switch (cellState) {
        case 0b00000000:
            color = EMPTY;
            break;
        case 0b00000001:
            color = BLACK;
            break;
        case 0b00000010:
            color = WHITE;
            break;
        default:
            perror("game-player : message.c : charToCell() : \nError: Received incompatible cell value\n");
            printf("Value is %x\n, should be either 0x00, 0x01 or 0x02\n", (unsigned char)cellState);
            color = (Color)-1;
    }
    return color;
}
