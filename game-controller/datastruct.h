#ifndef DATASTRUCT_H_INCLUDED
#define DATASTRUCT_H_INCLUDED

#include <stdlib.h>

//Identify message types
typedef enum {CONTROL, STATUS1, STATUS2, PING, END} MessageType;

//Identify player and cells color
typedef enum {WHITE, BLACK, EMPTY} Color;

//Coordinates data structure
typedef struct Coordinates {
    unsigned short x;
    unsigned short y;
} Coords;

//Board data structure
typedef struct BoardStructure {
    Coords *lastMove;
    Coords *dimensions;
    Color **state;
} Board;
void freeBoard(Board *board);

//Player data structure
typedef struct PlayerDataStructure {
    unsigned int points;
    unsigned int timer;
    unsigned int timerMSB;
    unsigned int timerLSB;
    char *playerName;
} PlayerData;

typedef struct PlayersDataStructure {
    PlayerData *dataWP;
    PlayerData *dataBP;
} PlayersData;

//Control data structure
typedef struct ControlDataStructure {
    Coords *newBoardSize;
    char gameMode;
    char restart;
} ControlData;

//Data to send to a message -> structure can be different according to the message, so we use a union
typedef union DataToSend {
    ControlData *controlData;   //For CONTROL
} MessageDataSend;

//Data to read from a message -> structure can be different according to the message, so we use a union
typedef union DataToRead {
    PlayersData *playersData;    //For STATUS2
    Board *board;   //For STATUS1
} MessageDataRead;

#endif // DATASTRUCT_H_INCLUDED
