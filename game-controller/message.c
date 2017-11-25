#include "message.h"

#define synchroValue 0x55

char *createMessage(MessageType messageType, MessageDataSend *data) {
    char *messageString = NULL;
    int length = 0;

    char type;
    char *content;
    char checksum = 0;

    switch (messageType) {
    case CONTROL:
        if (data == NULL || data->controlData == NULL) {
            perror("game-player : message.c : createMessage() :\nError: Received NULL DataToSend structure for a NEW_MOVE message\n");
            return NULL;
        }
        length = 4;
        type = 0x08;
        //Allocate char array to store the message content
        content = (char*)malloc(length*sizeof(char));
        //Fill the data
        content[0] = (char)data->controlData->newBoardSize->x;
        content[1] = (char)data->controlData->newBoardSize->y;
        content[2] = data->controlData->gameMode;
        content[3] = data->controlData->restart;
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
    if (length > 0) free(content);

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
    case 0x04:
        messageType = END;
        break;

    case 0x06:
        messageType = STATUS1;
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
        data->board->state = (Color**)malloc(data->board->dimensions->y*sizeof(Color*));
        for (i=0; i<data->board->dimensions->y; i++) {
            data->board->state[i] = (Color*)malloc(data->board->dimensions->x*sizeof(Color));
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
                char cellState = (messageBoardState[dataChar]&factor)>>byteDivider;
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

    case 0x07:
        messageType = STATUS2;
        data = (MessageDataRead*)malloc(sizeof(MessageDataRead));
        //Received the players data : allocate the data structure
        data->playersData = (PlayersData*)malloc(sizeof(PlayersData));
        data->playersData->dataWP = (PlayerData*)malloc(sizeof(PlayersData));
        data->playersData->dataBP = (PlayerData*)malloc(sizeof(PlayersData));
        //Fill the players Data
        PlayerData *pdata = data->playersData->dataBP;
        unsigned short adder = 0, cpt=0;
        for (cpt=0; cpt<2; cpt++) {
            pdata->points = content[adder];
            pdata->timerMSB = content[adder+1];
            pdata->timerLSB = content[adder+2];
            unsigned short playerNameLen = (unsigned short)content[adder+3];
            pdata->playerName = (char*)malloc((playerNameLen+1)*sizeof(char));
            int i;
            for (i=0; i<playerNameLen; i++) {
                pdata->playerName[i] = content[adder+i+4];
            }
            adder = playerNameLen + 4;
            pdata = data->playersData->dataWP;
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

Color charToCellColor(char cellState) {
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
            color = (Color)-1;
    }
    return color;
}
