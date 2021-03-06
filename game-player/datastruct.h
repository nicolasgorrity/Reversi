#ifndef DATASTRUCT_H_INCLUDED
#define DATASTRUCT_H_INCLUDED

#include <stdlib.h>

//Identify message types
typedef enum {OK, NOK, INIT_OK, CONNECT, NEXT_TURN, NEW_MOVE, END, PING} MessageType;

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
Board* copyBoard(Board *board);

typedef struct StringStruct {
    char *text;
    unsigned short length;
} String;
String* newString(char *text, unsigned short length);
void freeString(String *mystring);

//Data to send to a message -> structure can be different according to the message, so we use a union
typedef union DataToSend {
    Coords *newMoveCoords; //For NEW_MOVE
    String *playerName;      //For CONNECT
} MessageDataSend;

//Data to read from a message -> structure can be different according to the message, so we use a union
typedef union DataToRead {
    Color playerColor;    //For Player OK
    Board *board;   //For NEXT_TURN
} MessageDataRead;

//Single chained list describing an array of cells that can be taken to the opponent
typedef struct OpponentArrayOfCells {
    Coords *direction;
    unsigned short nbCellsTaken;
    struct OpponentArrayOfCells *next;
} OpponentArray;

//Single chained list to contain all the information about a playable cell
typedef struct PlayableCellStruct {
    Coords *cellCoords;
    unsigned short totalCellsTaken;
    unsigned short nbOfArrays;
    OpponentArray *opponentArrays;
    struct PlayableCellStruct *next;
} PlayableCell;
void freePlayableCell(PlayableCell *toDelete);
void freePlayableCells(PlayableCell *playableCells);

//Player turns single chained list for IA
typedef struct PlayerTurnStruct {
    Coords* cellCoords;
    Color player;
    struct PlayerTurnStruct *next;
} PlayerTurn;

//Node structure for IA
typedef struct NodeStruct {
    Coords *cellCoords;
    int evaluation;
} Node;

#endif // DATASTRUCT_H_INCLUDED
