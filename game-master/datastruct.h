#ifndef DATASTRUCT_H_INCLUDED
#define DATASTRUCT_H_INCLUDED

//Identify message types
typedef enum {OK, NOK, INIT_OK, CONNECT, NEXT_TURN, NEW_MOVE, END} MessageType;

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

//Data to send to a message -> structure can be different according to the message, so we use a union
typedef union DataToSend {
    Color playerColor;    //For Player OK
    Board *board;   //For NEXT_TURN
} MessageDataSend;

//Data to read from a message -> structure can be different according to the message, so we use a union
typedef union DataToRead {
    Coords *newMoveCoords; //For NEW_MOVE
    char *playerName;      //For CONNECT
} MessageDataRead;

#endif // DATASTRUCT_H_INCLUDED
