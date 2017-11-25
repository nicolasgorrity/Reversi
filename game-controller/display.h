#ifndef DISPLAY_H_INCLUDED
#define DISPLAY_H_INCLUDED

#include <stdio.h>

#include "datastruct.h"

void displayAllData(Board *board, PlayersData *playersData);
void displayBoard(Board *board);
void displayPlayersData(PlayersData *playersData);

char colorToSymbol(Color color);

#endif // DISPLAY_H_INCLUDED
