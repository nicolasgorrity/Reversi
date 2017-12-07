#ifndef PLAY_H_INCLUDED
#define PLAY_H_INCLUDED

#include <stdio.h>

#include "datastruct.h"

char isMoveValid(Board *board, Coords *newMove, Color playerColor);
int findSurroundedCellsArray(Board *board, int cell_x, int cell_y, int dir_h, int dir_v, Color playerColor, Color opponentColor);
unsigned short getNumberOfRemainingMoves(Board *board, Color playerColor);

void updateBoard(Board *board, Coords *newMove, Color playerColor);
Coords* calculateScores(Board *board, unsigned int whiteCumulatedTime, unsigned int blackCumulatedTime);

Color getOpponentColor(Color playerColor);

short isCellXInBoard(Board *board, int cell_x);
short isCellYInBoard(Board *board, int cell_y);
short isCellInBoard(Board *board, int cell_x, int cell_y);

#endif // PLAY_H_INCLUDED
