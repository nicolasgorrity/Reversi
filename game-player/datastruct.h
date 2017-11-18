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
struct Board {
    Coords *lastMove;
    Coords *dimensions;
    unsigned char **state;
}

//Data to send to a message -> structure can be different according to the message,
//so we use a union
typedef union DataToSend {
    Coords *coords; //For NEW_MOVE
    char *name;     //For CONNECT
} MessageDataSend;

typedef union DataToRead {
    Color color;    //For Player OK
    Board *board;   //For NEXT_TURN
} MessageDataRead;

#endif // DATASTRUCT_H_INCLUDED
