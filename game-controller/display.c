#include "display.h"

void displayAllData(Board *board, PlayersData *playersData) {
    displayPlayersData(playersData);
    displayBoard(board);
    displayCommands();
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
    int spaceTab = 20;
    int leftTab = 10;
    printf("\n%-*s%-*s%-*s\n", leftTab, "Player:", spaceTab, "Black", spaceTab, "White");
    printf("%-*s%-*s%-*s\n", leftTab, "Name:", spaceTab, playersData->dataBP->playerName, spaceTab, playersData->dataWP->playerName);
    printf("%-*s%-*s%-*s\n", leftTab, "Last move:", spaceTab, "", spaceTab, "");
    printf("%-*s%-*d%-*d\n", leftTab, "Points:", spaceTab, playersData->dataBP->points, spaceTab, playersData->dataWP->points);
    printf("%-*s%-*d%-*d\n", leftTab, "Time:", spaceTab, playersData->dataBP->timer, spaceTab, playersData->dataWP->timer);
}

void displayCommands() {
    printf("Commands: (s) step by step mode\n%10s(c) continuous mode\n%10s(ch) change board size\n%10s(q) quit\n%10s(r) restart\n","","","","");
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
