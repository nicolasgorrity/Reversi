#ifndef DATASTRUCT_H_INCLUDED
#define DATASTRUCT_H_INCLUDED

#include <stdlib.h>

//Identify message types
typedef enum {OK, NOK, INIT_OK, CONNECT, NEXT_TURN, NEW_MOVE, END, PING, STATUS1, STATUS2, CONTROL} MessageType;

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

Board* allocateInitialBoard(unsigned short sizeX, unsigned short sizeY);

typedef struct StringStruct {
    char *text;
    unsigned short length;
} String;
String* newString(char *text, unsigned short length);
void freeString(String *mystring);

typedef struct ControlDataStruct {
    Coords *newBoardSize;
    char restart;
    char mode;
} ControlData;

//Player data structure
typedef struct PlayerDataStructure {
    unsigned int points;
    unsigned short timer;
    String *playerName;
} PlayerData;

typedef struct PlayersDataStructure {
    PlayerData *dataWP;
    PlayerData *dataBP;
} PlayersData;
void freePlayersData(PlayersData *playersData);

//Data to send to a message -> structure can be different according to the message, so we use a union
typedef union DataToSend {
    Color playerColor;    //For Player OK
    Board *board;   //For NEXT_TURN and STATUS1
    PlayersData *playersData;   //For STATUS2
} MessageDataSend;

//Data to read from a message -> structure can be different according to the message, so we use a union
typedef union DataToRead {
    Coords *newMoveCoords; //For NEW_MOVE
    String *playerName;      //For CONNECT
    ControlData *control;
} MessageDataRead;

#endif // DATASTRUCT_H_INCLUDED
