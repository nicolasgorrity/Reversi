#ifndef ARTIFICIAL_INTELLIGENCE_H_INCLUDED
#define ARTIFICIAL_INTELLIGENCE_H_INCLUDED

#include "datastruct.h"
#include "play.h"

Coords* findBestMove(Board *board, Color playerColor);
Node* minimax(Board *board, PlayerTurn *playerTurns, unsigned short depth, Color playerColor, char isMaximizing, int **heuristicTable);
int heuristic(Board *board, Coords *coords, Color playerColor, int **heuristicTable);
int** heuristicBoard(Board *board);
int evaluation(Board *board, unsigned short x, unsigned short y, int **heuristicTable, Color playerColor);

#endif // ARTIFICIAL_INTELLIGENCE_H_INCLUDED
