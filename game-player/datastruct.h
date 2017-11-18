#ifndef DATASTRUCT_H_INCLUDED
#define DATASTRUCT_H_INCLUDED

//Identify message types
typedef enum {OK, NOK, INIT_OK, CONNECT, NEXT_TURN, NEW_MOVE, END} MessageType;

//Identify player color
typedef enum {WHITE, BLACK} PlayerColor;

//Coordinates data structure
typedef struct Coordinates {
    unsigned short x;
    unsigned short y;
} Coords;

//Board data structure
typedef struct BoardStructure {
    Coords *lastMove;
    Coords *dimensions;
    char **state;
} Board;

//Data to send to a message -> structure can be different according to the message,
//so we use a union
typedef union DataToSend {
    Coords *newMoveCoords; //For NEW_MOVE
    char *playerName;      //For CONNECT
} MessageDataSend;

typedef union DataToRead {
    PlayerColor playerColor;    //For Player OK
    Board *board;   //For NEXT_TURN
} MessageDataRead;

#endif // DATASTRUCT_H_INCLUDED
