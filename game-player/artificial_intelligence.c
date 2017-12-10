#include "artificial_intelligence.h"

Coords* findBestMove(Board *board, Color playerColor) {
    Coords* bestMoveCopy = (Coords*)malloc(sizeof(Coords));

    if (1) { //ALWAYS USE MINIMAX IA
        ///Get the heuristic table
        int **heuristicTable = heuristicBoard(board);
        ///Define the depth of the tree: here we predict 5 moves in advance (opponent's moves included)
        unsigned short depth = 5;
        ///Find the best move with MINIMAX algorithm
        Node *bestMove = minimax(board, NULL, depth, playerColor, 1, heuristicTable);
        ///Store the coordinates of the best move
        bestMoveCopy->x = bestMove->cellCoords->x;
        bestMoveCopy->y = bestMove->cellCoords->y;
        ///Free IA tools
        free(bestMove);
        int i;
        for (i=0; i<board->dimensions->y; i++) free(heuristicTable[i]);
        free(heuristicTable);
    }
    else {  //THIS IS THE METHOD WITHOUT IA. PLAYABLECELLS CONTAIN A LOT OF INFORMATION SO WE CAN STILL DO INTERESTING THINGS, BUT NOT ANTICIPATE THE FUTURE MOVES
        ///Get the chained list of playable cells
        PlayableCell *playableCells = findPlayableCells(board, playerColor);

        ///Chose the best move
        //For now, we simply send the first one
        Coords* bestMove;
        if (playableCells != NULL) {
            bestMove = playableCells->cellCoords;
        }
        else {
            bestMove = (Coords*)malloc(sizeof(Coords));
            bestMove->x = USHRT_MAX;
            bestMove->y = USHRT_MAX;
        }

        ///Copy its data into a Coords* data structure
        bestMoveCopy->x = bestMove->x;
        bestMoveCopy->y = bestMove->y;

        ///Free the list of playable cells
        freePlayableCells(playableCells);
    }

    ///Return the coords of the best move
    return bestMoveCopy;
}

Node* minimax(Board *board, PlayerTurn *playerTurns, unsigned short depth, Color playerColor, char isMaximizing, int **heuristicTable) {
    //Create a copy of the board to manipulate it without loosing the original one
    Board *boardCpy = copyBoard(board);

    Node *node = (Node*)malloc(sizeof(Node));
    node->cellCoords = (Coords*)malloc(sizeof(Coords));
    node->cellCoords->x = USHRT_MAX; //if no possible move, aberrant values will be returned
    node->cellCoords->y = USHRT_MAX;

    //Redo the previous anticipated moves
    PlayerTurn *turnTmp = playerTurns;
    PlayerTurn *lastMove = NULL;
    while (turnTmp != NULL) {
        if (turnTmp->next == NULL) lastMove = turnTmp;
        updateBoard(boardCpy, turnTmp->cellCoords, turnTmp->player);
        turnTmp = turnTmp->next;
    }

    ///For a leaf
    if (depth == 0) {
        node->cellCoords->x = lastMove->cellCoords->x;
        node->cellCoords->y = lastMove->cellCoords->y;
        node->evaluation = heuristic(boardCpy, node->cellCoords, isMaximizing ? playerColor : getOpponentColor(playerColor), heuristicTable);
        return node;
    }

    ///For a maximizing node
    if (isMaximizing) {
        //Defining minimum
        node->evaluation = INT_MIN;
        //Search child nodes
        PlayableCell *playableCells = findPlayableCells(boardCpy, playerColor);
        //If no playable move for player, should avoid this node ? (evaluation = INT_MIN)
        if (playableCells == NULL) {
            node->evaluation = heuristic(boardCpy, node->cellCoords, playerColor, heuristicTable);
            if (lastMove != NULL) {
                node->cellCoords->x = lastMove->cellCoords->x;
                node->cellCoords->y = lastMove->cellCoords->y;
            }
        }
        //Browse child nodes
        PlayableCell *childNodeTmp = playableCells;
        while (childNodeTmp != NULL) {
            PlayerTurn *childTurn = (PlayerTurn*)malloc(sizeof(PlayerTurn));
            childTurn->cellCoords = childNodeTmp->cellCoords;
            childTurn->player = playerColor;
            childTurn->next = NULL;
            if (lastMove == NULL) playerTurns = childTurn;
            else lastMove->next = childTurn;
            Node *childResult = minimax(board, playerTurns, depth-1, playerColor, 0, heuristicTable);
            if (childResult->evaluation > node->evaluation) {
                node->evaluation = childResult->evaluation;
                node->cellCoords->x = childNodeTmp->cellCoords->x;
                node->cellCoords->y = childNodeTmp->cellCoords->y;
            }
            PlayableCell *toDelete = childNodeTmp;
            childNodeTmp = childNodeTmp->next;
            freePlayableCell(toDelete);
            free(childTurn);
            free(childResult->cellCoords);
            free(childResult);
        }
        return node;
    }

    ///For a minimizing node
    else {
        //Defining maximum
        node->evaluation = INT_MAX;
        //Search child nodes
        Color opponentColor = getOpponentColor(playerColor);
        PlayableCell *playableCells = findPlayableCells(boardCpy, opponentColor);
        //If no playable move for opponent, should keep this node ? (evaluation = INT_MAX) --> this will valorize moves blocking the opponent
        if (playableCells == NULL) {
            node->evaluation = heuristic(boardCpy, node->cellCoords, playerColor, heuristicTable);
            if (lastMove != NULL) {
                node->cellCoords->x = lastMove->cellCoords->x;
                node->cellCoords->y = lastMove->cellCoords->y;
            }
        }
        //Browse child nodes
        PlayableCell *childNodeTmp = playableCells;
        while (childNodeTmp != NULL) {
            PlayerTurn *childTurn = (PlayerTurn*)malloc(sizeof(PlayerTurn));
            childTurn->cellCoords = childNodeTmp->cellCoords;
            childTurn->player = opponentColor;
            childTurn->next = NULL;
            if (lastMove == NULL) playerTurns = childTurn;
            else lastMove->next = childTurn;
            Node *childResult = minimax(board, playerTurns, depth-1, playerColor, 1, heuristicTable);
            if (childResult->evaluation < node->evaluation) {
                node->evaluation = childResult->evaluation;
                node->cellCoords->x = childNodeTmp->cellCoords->x;
                node->cellCoords->y = childNodeTmp->cellCoords->y;
            }
            PlayableCell *toDelete = childNodeTmp;
            childNodeTmp = childNodeTmp->next;
            freePlayableCell(toDelete);
            free(childTurn);
            free(childResult->cellCoords);
            free(childResult);
        }
        return node;
    }
    return NULL;
}

int heuristic(Board *board, Coords *coords, Color playerColor, int **heuristicTable) {
    unsigned char K_mobility, K_strength, K_quantity;
    ///Calculate the QUANTITY criteria: number of coins
    unsigned short quantity = 0;
    unsigned short totalCoins = 0;
    ///In the same time, calculate the STRENGTH criteria: the value of each cell
    int strength = 0;
    unsigned short i,j;
    for (i=0; i<board->dimensions->y; i++) {
        for (j=0; j<board->dimensions->x; j++) {
            if (board->state[i][j] == playerColor) {
                quantity++;
                strength += evaluation(board, j, i, heuristicTable, playerColor);
            }
            if (board->state[i][j] != EMPTY) {
                totalCoins++;
            }
        }
    }

    ///Calculate the MOBILITY criteria: number of playable cells
    unsigned short mobility = getNumberOfRemainingMoves(board, playerColor);

    ///Define the criterias coefficients
    unsigned short boardSize = board->dimensions->x * board->dimensions->y;
    //Beginning
    if (totalCoins <= boardSize / 4) {
        K_mobility = 35;
        K_strength = 20;
        K_quantity = 5;
    }
    //End
    else if (boardSize - totalCoins <= boardSize / 4) {
        K_mobility = 10;
        K_strength = 10;
        K_quantity = 40;
    }
    //Middle
    else {
        K_mobility = 20;
        K_strength = 30;
        K_quantity = 10;
    }

    return K_mobility*mobility + K_strength*strength + K_quantity*quantity;
}

int** heuristicBoard(Board *board) {
    int **boardCpy = (int**)malloc(board->dimensions->y * sizeof(int*));
    int i,j;
    for (i=0; i<board->dimensions->y; i++) {
        boardCpy[i] = (int*)malloc(board->dimensions->x * sizeof(int));
        for (j=0; j<board->dimensions->x; j++) {
            boardCpy[i][j] = 0;
            //Corners are great
            if (i==1 || i==board->dimensions->y-2) {
                if (j==0 || j==board->dimensions->x-1) boardCpy[i][j] = -150;
                else if (j==1 || j==board->dimensions->x-2) boardCpy[i][j] = -250;
            }
            else if (i==0 || i==board->dimensions->y-1) {
                boardCpy[i][j] = 10;
                if (j==2 || j==board->dimensions->x-3) boardCpy[i][j] = 30;
                if (j==1 || j==board->dimensions->x-2) boardCpy[i][j] = -150;
                if (j==0 || j==board->dimensions->x-1) boardCpy[i][j] = 500;
            }
            else if (j==0 || j==board->dimensions->x-1) {
                if (boardCpy[i][j] == 0) {
                    boardCpy[i][j] = 10;
                    if (i==2 || i==board->dimensions->x-3) boardCpy[i][j] = 30;
                }
            }
            else { //Middle of table
                unsigned short midX = board->dimensions->x / 2;
                unsigned short midY = board->dimensions->y / 2;
                if ((i==midY-2 || i==board->dimensions->y-midY+1) && (j==midX-2 || j==board->dimensions->x-midX+1)) boardCpy[i][j] = 1;
                else if (((i==midY-2 || i==board->dimensions->y-midY+1) && (j>=midX-1 && j<=board->dimensions->x-midX))
                        || ((i>=midY-1 && i<=board->dimensions->y-midY) && (j==midX-2 || j==board->dimensions->x-midX+1))) {
                    boardCpy[i][j] = 2;
                }
                else if ((i>=midY-1 && i<=board->dimensions->y-midY) && (j>=midX-1 && j<=board->dimensions->x-midX)) boardCpy[i][j] = 16;
            }
        }
    }

    return boardCpy;
}

int evaluation(Board *board, unsigned short x, unsigned short y, int **heuristicTable, Color playerColor) {
    ///Get the value of the cell in the heuristic table
    int cellValue = heuristicTable[y][x];
    ///Adjust according to board composition:
    if (board->state[0][0]==playerColor) {  //top left
        //if corner already taken, adjacent cells are not bad anymore
        if ((x+y==1) || (x==1 && y==1)) cellValue = 90;
        //if on adjacent side, more interesting if corner taken
        if (y==0 && x>1 && x<board->dimensions->x-2) { //top side
            unsigned short cpt=0;
            while (x != cpt) {
                cpt++;
                if (board->state[y][x-cpt] != playerColor) {
                    cpt = -1;
                    break;
                }
            }
            if (cpt > 0) { //line of allies cells until corner
                cellValue = 70;
            }
        }
        if (x==0 && y>1 && y<board->dimensions->y-2) { //left side
            unsigned short cpt=0;
            while (y != cpt) {
                cpt++;
                if (board->state[y-cpt][x] != playerColor) {
                    cpt = -1;
                    break;
                }
            }
            if (cpt > 0) { //line of allies cells until corner
                cellValue = 70;
            }
        }
    }
    if (board->state[board->dimensions->y-1][0]==playerColor) { //bottom left
        //if corner already taken, adjacent cells are not bad anymore
        if (((x==0 || x==1) && y==board->dimensions->y-2) || (x==1 && y==board->dimensions->y-1)) cellValue = 90;
        //if on adjacent side, more interesting if corner taken
        if (y==board->dimensions->y-1 && x>1 && x<board->dimensions->x-2) { //bottom side
            unsigned short cpt=0;
            while (x != cpt) {
                cpt++;
                if (board->state[y][x-cpt] != playerColor) {
                    cpt = -1;
                    break;
                }
            }
            if (cpt > 0) { //line of allies cells until corner
                cellValue = 70;
            }
        }
        if (x==0 && y>1 && y<board->dimensions->y-2) { //left side
            unsigned short cpt=0;
            while (y+cpt != board->dimensions->y-1) {
                cpt++;
                if (board->state[y+cpt][x] != playerColor) {
                    cpt = -1;
                    break;
                }
            }
            if (cpt > 0) { //line of allies cells until corner
                cellValue = 70;
            }
        }
    }
    if (board->state[0][board->dimensions->x-1]==playerColor) { //top right
        //if corner already taken, adjacent cells are not bad anymore
        if (((y==0 || y==1) && x==board->dimensions->x-2) || (x==board->dimensions->x-1 && y==1)) cellValue = 90;
        //if on adjacent side, more interesting if corner taken
        if (y==0 && x>1 && x<board->dimensions->x-2) { //top side
            unsigned short cpt=0;
            while (x+cpt != board->dimensions->x-1) {
                cpt++;
                if (board->state[y][x+cpt] != playerColor) {
                    cpt = -1;
                    break;
                }
            }
            if (cpt > 0) { //line of allies cells until corner
                cellValue = 70;
            }
        }
        if (x==board->dimensions->x-1 && y>1 && y<board->dimensions->y-2) { //right side
            unsigned short cpt=0;
            while (y != cpt) {
                cpt++;
                if (board->state[y-cpt][x] != playerColor) {
                    cpt = -1;
                    break;
                }
            }
            if (cpt > 0) { //line of allies cells until corner
                cellValue = 70;
            }
        }
    }
    if (board->state[board->dimensions->y-1][board->dimensions->x-1]==playerColor) { //bottom right
        //if corner already taken, adjacent cells are not bad anymore
        if (((y==board->dimensions->y-2 || y==board->dimensions->y-1) && x==board->dimensions->x-2) || (x==board->dimensions->x-1 && y==board->dimensions->y-2)) cellValue = 90;
        //if on adjacent side, more interesting if corner taken
        if (y==board->dimensions->y-1 && x>1 && x<board->dimensions->x-2) { //bottom side
            unsigned short cpt=0;
            while (x+cpt != board->dimensions->x-1) {
                cpt++;
                if (board->state[y][x+cpt] != playerColor) {
                    cpt = -1;
                    break;
                }
            }
            if (cpt > 0) { //line of allies cells until corner
                cellValue = 70;
            }
        }
        if (x==board->dimensions->x-1 && y>1 && y<board->dimensions->y-2) { //right side
            unsigned short cpt=0;
            while (y+cpt != board->dimensions->y-1) {
                cpt++;
                if (board->state[y+cpt][x] != playerColor) {
                    cpt = -1;
                    break;
                }
            }
            if (cpt > 0) { //line of allies cells until corner
                cellValue = 70;
            }
        }
    }

    return cellValue;
}
