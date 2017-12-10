#include "datastruct.h"

void freeBoard(Board *board) {
    int i;
    for (i=0; i<board->dimensions->y; i++) {
        free(board->state[i]);
    }
    free(board->state);
    free(board->dimensions);
    free(board->lastMove);
}


Board* copyBoard(Board *board) {
    Board *boardCpy = (Board*)malloc(sizeof(Board));
    boardCpy->dimensions = (Coords*)malloc(sizeof(Coords));
    boardCpy->dimensions->x = board->dimensions->x;
    boardCpy->dimensions->y = board->dimensions->y;

    boardCpy->lastMove = (Coords*)malloc(sizeof(Coords));
    boardCpy->lastMove->x = board->lastMove->x;
    boardCpy->lastMove->y = board->lastMove->y;

    boardCpy->state = (Color**)malloc(board->dimensions->y * sizeof(Color*));
    int i,j;
    for (i=0; i<board->dimensions->y; i++) {
        boardCpy->state[i] = (Color*)malloc(board->dimensions->x * sizeof(Color));
        for (j=0; j<board->dimensions->x; j++) {
            boardCpy->state[i][j] = board->state[i][j];
        }
    }

    return boardCpy;
}

void freePlayableCell(PlayableCell *toDelete) {
    free(toDelete->cellCoords);
    //Free the list of arrays
    while (toDelete->opponentArrays != NULL) {
        OpponentArray *toFree = toDelete->opponentArrays;
        toDelete->opponentArrays = toDelete->opponentArrays->next;
        free(toFree->direction);
        free(toFree);
    }
    free(toDelete);
}

void freePlayableCells(PlayableCell *playableCells) {
    while (playableCells != NULL) {
        PlayableCell *toDelete = playableCells;
        playableCells = playableCells->next;
        freePlayableCell(toDelete);
    }
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
