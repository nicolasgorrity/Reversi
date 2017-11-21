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
