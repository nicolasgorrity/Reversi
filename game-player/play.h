#ifndef PLAY_H_INCLUDED
#define PLAY_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "datastruct.h"

PlayableCell* findPlayableCells(Board *board, Color playerColor);
int findSurroundedCellsArray(Board *board, int cell_x, int cell_y, int dir_h, int dir_v, Color playerColor, Color opponentColor);

Color getOpponentColor(Color playerColor);

short isCellXInBoard(Board *board, int cell_x);
short isCellYInBoard(Board *board, int cell_y);
short isCellInBoard(Board *board, int cell_x, int cell_y);

void updateBoard(Board *board, Coords *newMove, Color playerColor);

unsigned short getNumberOfRemainingMoves(Board *board, Color playerColor);

#endif // PLAY_H_INCLUDED
