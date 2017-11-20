#ifndef PLAY_H_INCLUDED
#define PLAY_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>

#include "datastruct.h"

Coords* findBestMove(Board *board, Color playerColor);
PlayableCell* findPlayableCells(Board *board, Color playerColor);
int findSurroundedCellsArray(Board *board, int cell_x, int cell_y, int dir_h, int dir_v, Color playerColor, Color opponentColor);

Color getOpponentColor(Color playerColor);

short isCellXInBoard(Board *board, int cell_x);
short isCellYInBoard(Board *board, int cell_y);
short isCellInBoard(Board *board, int cell_x, int cell_y);


#endif // PLAY_H_INCLUDED
