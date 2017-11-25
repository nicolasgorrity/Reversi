#include "display.h"

void displayAllData(Board *board, PlayersData *playersData) {
    displayPlayersData(playersData);
    displayBoard(board);
}

void displayBoard(Board *board) {
    printf("\n%-2s"," ");
    int i,j;
    for (j=0; j<board->dimensions->x; j++) {
        printf("%-2c", 'A'+j);
    }
    printf("\n");
    for (i=0; i<board->dimensions->y; i++) {
        printf("%-2d",i+1);
        for (j=0; j<board->dimensions->x; j++) {
            printf("%-2c", colorToSymbol(board->state[i][j]));
        }
        printf("\n");
    }

}

void displayPlayersData(PlayersData *playersData) {

}

char colorToSymbol(Color color) {
    char symbol;
    switch(color) {
    case BLACK:
        symbol = 'B';
        break;
    case WHITE:
        symbol = 'W';
        break;
    case EMPTY:
        symbol = '_';
        break;
    default:
        symbol = '?';
    }
    return symbol;
}
