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

//Data to send to a message -> structure can be different according to the message, so we use a union
typedef union DataToSend {
    Color playerColor;    //For Player OK
    Board *board;   //For NEXT_TURN
} MessageDataSend;

//Data to read from a message -> structure can be different according to the message, so we use a union
typedef union DataToRead {
    Coords *newMoveCoords; //For NEW_MOVE
    String *playerName;      //For CONNECT
} MessageDataRead;

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

#endif // DATASTRUCT_H_INCLUDED
