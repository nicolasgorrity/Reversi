#include "play.h"

Coords* calculateScores(Board *board, unsigned int whiteCumulatedTime, unsigned int blackCumulatedTime) {
    unsigned short whiteCoins = 0;
    unsigned short blackCoins = 0;
    ///Count the number of coins of each player
    unsigned short i,j;
    for (i=0; i<board->dimensions->y; i++) {
        for (j=0; j<board->dimensions->x; j++) {
            if (board->state[i][j] == WHITE) whiteCoins++;
            else if (board->state[i][j] == BLACK) blackCoins++;
        }
    }
    unsigned short totalCoins = whiteCoins + blackCoins;

    unsigned int totalCumulatedTime = whiteCumulatedTime + blackCumulatedTime;

    ///Calculate eventual bonus
    unsigned short whiteBonus = ((whiteCoins > blackCoins) ? 100 : 0);
    unsigned short blackBonus = ((blackCoins > whiteCoins) ? 100 : 0);

    ///Calculate the white score
    unsigned short whiteScore = (whiteCoins/totalCoins)*100 + ((totalCumulatedTime-whiteCumulatedTime) / totalCumulatedTime)*100 + whiteBonus;
    unsigned short blackScore = (blackCoins/totalCoins)*100 + ((totalCumulatedTime-blackCumulatedTime) / totalCumulatedTime)*100 + blackBonus;

    Coords *scores = (Coords*)malloc(sizeof(Coords));
    scores->x = whiteScore;
    scores->y = blackScore;

    return scores;
}

void updateBoard(Board *board, Coords *newMove, Color playerColor) {
    ///Check that the new move is inside the board
    if (!isCellInBoard(board, newMove->x, newMove->y)) {
        printf("Move not possible for %s player\n", (playerColor == WHITE ? "White" : "Black"));
        return;
    }

    ///Find the opponent color
    Color opponentColor = getOpponentColor(playerColor);
    if (opponentColor == (Color)-1) {
        perror("game-player : play.c : updateBoard() :\nError: Couldn't find the opponent color.\n");
        return;
    }

    ///The cell has to be empty
    if (board->state[newMove->y][newMove->x] == EMPTY) {
        ///The cell has to be adjacent to an opponent cell
        int dir_v, dir_h;
        for (dir_v=-1; dir_v<=1; dir_v++) {
            //Be sure that we are not checking outside the board
            if (isCellYInBoard(board, newMove->y+dir_v)) {
                for (dir_h=-1; dir_h<=1; dir_h++) {
                    //Be sure that we are not checking outside the board
                    if (isCellXInBoard(board, newMove->x+dir_h)) {
                        if (board->state[newMove->y+dir_v][newMove->x+dir_h] == opponentColor) {
                            ///Following the same direction, we should find several (min 1) opponent cells, ended by a cell belonging to the current player
                            int surroundedArraySize = findSurroundedCellsArray(board, newMove->x+dir_h, newMove->y+dir_v, dir_h, dir_v, playerColor, opponentColor);
                            if (surroundedArraySize > 0) { //Array of 1+ opponent cells ended by a current player's cell: this is a playable move
                                ///This array is taken from the opponent
                                int cpt = 0, r_i=0, r_j=0;
                                for (cpt = 0; cpt<surroundedArraySize+1; cpt++) {
                                    board->state[newMove->y+r_i][newMove->x+r_j] = playerColor;
                                    r_i += dir_v;
                                    r_j += dir_h;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

unsigned short getNumberOfRemainingMoves(Board *board, Color playerColor) {
    int i,j;
    int numberOfPlayableCells = 0;

    ///Find the opponent color
    Color opponentColor = getOpponentColor(playerColor);
    if (opponentColor == (Color)-1) {
        perror("game-player : play.c : getNumberOfRemainingMoves() :\nError: Couldn't find the opponent color.\n");
        return 0;
    }

    ///Browse the board
    for (i=0; i<board->dimensions->y; i++) {
        for (j=0; j<board->dimensions->x; j++) {
            ///The cell has to be empty
            if (board->state[i][j] == EMPTY) {
                char isPlayable=0;
                ///The cell has to be adjacent to an opponent cell
                int dir_v, dir_h;
                for (dir_v=-1; dir_v<=1; dir_v++) {
                    //Be sure that we are not checking outside the board
                    if (isCellYInBoard(board, i+dir_v)) {
                        for (dir_h=-1; dir_h<=1; dir_h++) {
                            //Be sure that we are not checking outside the board
                            if (isCellXInBoard(board, j+dir_h)) {
                                if (board->state[i+dir_v][j+dir_h] == opponentColor) {
                                    ///Following the same direction, we should find several (min 1) opponent cells, ended by a cell belonging to the current player
                                    int surroundedArraySize = findSurroundedCellsArray(board, j+dir_h, i+dir_v, dir_h, dir_v, playerColor, opponentColor);
                                    if (surroundedArraySize > 0) { //Array of 1+ opponent cells ended by a current player's cell: this is a playable move
                                        isPlayable = 1;
                                    }
                                }
                            }
                        }
                    }
                }
                numberOfPlayableCells += isPlayable;
            }
        }
    }
    return numberOfPlayableCells;
}

char isMoveValid(Board *board, Coords *newMove, Color playerColor) {
    ///Check that the new move is inside the board
    if (!isCellInBoard(board, newMove->x, newMove->y)) {
        printf("Move not possible for %s player\n", (playerColor == WHITE ? "White" : "Black"));
        return -1;
    }

    ///Find the opponent color
    Color opponentColor = getOpponentColor(playerColor);
    if (opponentColor == (Color)-1) {
        perror("game-player : play.c : findPlayableCells() :\nError: Couldn't find the opponent color.\n");
        return -1;
    }

    ///The cell has to be empty
    if (board->state[newMove->y][newMove->x] == EMPTY) {
        ///The cell has to be adjacent to an opponent cell
        int dir_v, dir_h;
        for (dir_v=-1; dir_v<=1; dir_v++) {
            //Be sure that we are not checking outside the board
            if (isCellYInBoard(board, newMove->y+dir_v)) {
                for (dir_h=-1; dir_h<=1; dir_h++) {
                    //Be sure that we are not checking outside the board
                    if (isCellXInBoard(board, newMove->x+dir_h)) {
                        if (board->state[newMove->y+dir_v][newMove->x+dir_h] == opponentColor) {
                            ///Following the same direction, we should find several (min 1) opponent cells, ended by a cell belonging to the current player
                            int surroundedArraySize = findSurroundedCellsArray(board, newMove->x+dir_h, newMove->y+dir_v, dir_h, dir_v, playerColor, opponentColor);
                            if (surroundedArraySize > 0) { //Array of 1+ opponent cells ended by a current player's cell: this is a playable move
                                ///If we find at least one valid array, the cell is playable
                                return 1;
                            }
                        }
                    }
                }
            }
        }
    }
    return 0;
}

int findSurroundedCellsArray(Board *board, int cell_x, int cell_y, int dir_h, int dir_v, Color playerColor, Color opponentColor) {
    //Check the cell is within the board limits
    if (!isCellInBoard(board, cell_x, cell_y)) return -1;

    Color cellColor = board->state[cell_y][cell_x];

    //An opponent cell is found: we keep searching in this direction
    if (cellColor == opponentColor) {
        int result = findSurroundedCellsArray(board, cell_x+dir_h, cell_y+dir_v, dir_h, dir_v, playerColor, opponentColor);
        if (result == -1) return -1; //No opponent cell found before finding an empty cell or the board limit
        return 1+result; //Return the number of opponent cells found after this one, plus this one
    }

    //A cell belonging to the current player is found, marking the end of the array
    if (cellColor == playerColor) {
        return 0;
    }

    //If no previous case has been raised, it means the current cell is neither WHITE or BLACK: the move is not possible
    if (cellColor != EMPTY) {
        perror("game-player : play.c : findSurroundedCellsArray() :\nError: Bad Color found in board: neither BLACK nor EMPTY nor WHITE\n");
    }
    return -1;
}

Color getOpponentColor(Color playerColor) {
    Color opponentColor;
    if (playerColor == BLACK) opponentColor = WHITE;
    else if (playerColor == WHITE) opponentColor = BLACK;
    else {
        perror("game-player : play.c : getOpponentColor() :\nError: Received wrong player color: neither WHITE nor BLACK.\n");
        opponentColor = (Color)-1;
    }
    return opponentColor;
}

short isCellXInBoard(Board *board, int cell_x) {
    return (cell_x>=0 && cell_x<board->dimensions->x);
}

short isCellYInBoard(Board *board, int cell_y) {
    return (cell_y>=0 && cell_y<board->dimensions->y);
}

short isCellInBoard(Board *board, int cell_x, int cell_y) {
    return (isCellXInBoard(board, cell_x) && isCellYInBoard(board, cell_y));
}
