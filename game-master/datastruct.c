#include "datastruct.h"
#include <stdio.h>
void freeBoard(Board *board) {
    int i;
    for (i=0; i<board->dimensions->y; i++) {
        free(board->state[i]);
    }
    free(board->state);
    free(board->dimensions);
    free(board->lastMove);
}

Board* allocateInitialBoard(unsigned short sizeX, unsigned short sizeY) {
    Board *board = (Board*)malloc(sizeof(Board));
    board->dimensions = (Coords*)malloc(sizeof(Coords));
    board->dimensions->x = sizeX;
    board->dimensions->y = sizeY;

    board->lastMove = (Coords*)malloc(sizeof(Coords));
    board->lastMove->x = 4;
    board->lastMove->y = 3;

    board->state = (Color**)malloc(sizeY * sizeof(Color*));
    int i,j;
    for (i=0; i<sizeY; i++) {
        board->state[i] = (Color*)malloc(sizeX * sizeof(Color));
        for (j=0; j<sizeX; j++) {
            if ((i==3 && j==4) || (i==4 && j==3)) board->state[i][j] = WHITE;
            else if ((i==4 && j==4) || (i==3 && j==3)) board->state[i][j] = BLACK;
            else board->state[i][j] = EMPTY;
        }
    }

    return board;
}

String* newString(char *text, unsigned short length) {
    String *string = (String*)malloc(sizeof(String));
    string->text = text;
    string->length = length;
    return string;
}

void freeString(String *mystring) {
    free(mystring->text);
    free(mystring);
}
