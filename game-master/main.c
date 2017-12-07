#include "connection.h"
#include "message.h"
#include "play.h"
#include "display.h"

Coords* requestPlayerMove(int playerSocket, Board *board) {
    ///Send NEXT-TURN message to player
    MessageDataSend *dataSend = (MessageDataSend*)malloc(sizeof(MessageDataSend));
    dataSend->board = board;
    char *nextTurnMsg = createMessage(NEXT_TURN, dataSend);
    free(dataSend);
    if(writeMessage(playerSocket, nextTurnMsg) < 0) {
        perror("game-master : main.c : requestPlayerMove() : Could not send NEXT_TURN message\n");
        free(nextTurnMsg);
        return NULL;
    }
    free(nextTurnMsg);

    ///Wait for NEW_MOVE message
    char *newMoveMsg = readMessage(playerSocket);
    MessageDataRead *dataRead = (MessageDataRead*)malloc(sizeof(MessageDataRead));
    if (extractMessage(newMoveMsg, dataRead) != NEW_MOVE) {
        perror("game-master : main.c : requestPlayerMove() : Could not read NEW_MOVE message\n");
        free(dataRead);
        free(newMoveMsg);
        return NULL;
    }
    free(newMoveMsg);

    Coords *newMove = dataRead->newMoveCoords;
    free(dataRead);

    return newMove;
}

int main(int argc, char *argv[])
{
    unsigned int portPlayer1 = 8888;
    unsigned int portPlayer2 = 8889;
    unsigned int portController = 8890;

    int socket1 = createSocket();
    int socket2 = createSocket();
    int socketC = createSocket();

    if (bindSocket(socket1, portPlayer1) < 0) return disconnect(socket1, socket2, socketC, -1, -1, -1);
    if (bindSocket(socket2, portPlayer2) < 0) return disconnect(socket1, socket2, socketC, -1, -1, -1);
    if (bindSocket(socketC, portController) < 0) return disconnect(socket1, socket2, socketC, -1, -1, -1);

    if (listenSocket(socket1) < 0) return disconnect(socket1, socket2, socketC, -1, -1, -1);
    int sockW = acceptSocket(socket1);
    if (sockW < 0) return disconnect(socket1, socket2, socketC, -1, -1, -1);

    if (listenSocket(socket2) < 0) return disconnect(socket1, socket2, socketC, sockW, -1, -1);
    int sockB = acceptSocket(socket2);
    if (sockB < 0) return disconnect(socket1, socket2, socketC, sockW, -1, -1);;

    int sockC = -1;
    /*if (listenSocket(socketC) < 0) return disconnect(socket1, socket2, socketC, sockW, sockB, -1);;
    int sockC = acceptSocket(socketC);
    if (sockC < 0) return disconnect(socket1, socket2, socketC, sockW, sockB, -1);;
*/
    ////////White player///////
    ///Wait for Connect message
    char *connectMsgW = readMessage(sockW);
    MessageDataRead *dataRead = (MessageDataRead*)malloc(sizeof(MessageDataRead));
    if (extractMessage(connectMsgW, dataRead) != CONNECT) {
        perror("game-master : main.c : Could not read CONNECT message\n");
        free(connectMsgW);
        free(dataRead);
        return disconnect(socket1, socket2, socketC, sockW, sockB, sockC);
    }
    char* whiteName = dataRead->playerName;
    free(dataRead);
    free(connectMsgW);

    ///Send OK message
    MessageDataSend *dataSend = (MessageDataSend*)malloc(sizeof(MessageDataSend));
    dataSend->playerColor = WHITE;
    free(dataSend);
    char *okMessage = createMessage(INIT_OK, dataSend);
    if(writeMessage(sockW, okMessage) < 0) {
        perror("game-master : main.c : Could not send OK message\n");
        free(whiteName);
        free(okMessage);
        return disconnect(socket1, socket2, socketC, sockW, sockB, sockC);
    }
    free(okMessage);

    ////////Black player///////
    ///Wait for Connect message
    char *connectMsgB = readMessage(sockB);
    dataRead = (MessageDataRead*)malloc(sizeof(MessageDataRead));
    if (extractMessage(connectMsgB, dataRead) != CONNECT) {
        perror("game-master : main.c : Could not read CONNECT message\n");
        free(whiteName);
        free(connectMsgB);
        free(dataRead);
        return disconnect(socket1, socket2, socketC, sockW, sockB, sockC);
    }
    char* blackName = dataRead->playerName;
    free(dataRead);
    free(connectMsgB);

    ///Send OK message
    dataSend = (MessageDataSend*)malloc(sizeof(MessageDataSend));
    dataSend->playerColor = BLACK;
    okMessage = createMessage(INIT_OK, dataSend);
    free(dataSend);
    if(writeMessage(sockB, okMessage) < 0) {
        perror("game-master : main.c : Could not send INIT_OK message\n");
        free(blackName);
        free(whiteName);
        free(okMessage);
        return disconnect(socket1, socket2, socketC, sockW, sockB, sockC);
    }
    free(okMessage);
    free(dataSend);

    ///////////////////////////////////////////
    ////////Message to Game Controller????/////
    ///////////////////////////////////////////

    Board *board = allocateInitialBoard(8, 8);

    ///Game Loop
    char exit = 0;
    Color hasLost = EMPTY;
    while (!exit) {
        ///Ask and receive new move from BP:
        Coords *newMove = requestPlayerMove(sockB, board);
        if (newMove == NULL || isMoveValid(board, newMove, BLACK) <= 0) {
            //Send NOK message
            okMessage = createMessage(NOK, NULL);
            if (writeMessage(sockB, okMessage) < 0) {
                perror("game-master : main.c : Could not send NOK message to BP\n");
                free(okMessage);
                freeBoard(board);
                free(blackName);
                free(whiteName);
                return disconnect(socket1, socket2, socketC, sockW, sockB, sockC);
            }
            free(okMessage);
            if (newMove != NULL) free(newMove);
            //Black player just lost
            hasLost = BLACK;
            exit = 1;
            continue;
        }
        ///Send OK message to BP
        okMessage = createMessage(OK, NULL);
        if (writeMessage(sockB, okMessage) < 0) {
            perror("game-master : main.c : Could not send OK message to BP\n");
            free(okMessage);
            freeBoard(board);
            free(blackName);
            free(whiteName);
            return disconnect(socket1, socket2, socketC, sockW, sockB, sockC);
        }
        free(okMessage);

        ///Update board with new move
        updateBoard(board, newMove, BLACK);
        free(newMove);

        ///Calculate scores
        Coords *scores = calculateScores(board, 1, 1); //returns two unsigned short : x white , y: black
        unsigned short whiteScore = scores->x;
        unsigned short blackScore = scores->y;
        free(scores);

        /////////////////////////////////////////////////
        //////////SEND NEW BOARD TO AFFICHAGE////////////
        PlayersData *playersData = (PlayersData*)malloc(sizeof(PlayerData));
        playersData->dataBP = (PlayerData*)malloc(sizeof(PlayerData));
        playersData->dataWP = (PlayerData*)malloc(sizeof(PlayerData));
        playersData->dataBP->playerName = "coucouBlackPlayer";
        playersData->dataWP->playerName = "coucouWhitePlayer";
        playersData->dataBP->points = whiteScore;
        playersData->dataWP->points = blackScore;
        playersData->dataBP->timer = 1;
        playersData->dataWP->timer = 1;
        displayAllData(board, playersData);
        free(playersData->dataBP);
        free(playersData->dataWP);
        free(playersData);
        ////////////////END AFFICHAGE////////////////////
        /////////////////////////////////////////////////

        ///Ask and receive new move from WP:
        newMove = requestPlayerMove(sockW, board);
        while (newMove == NULL || isMoveValid(board, newMove, WHITE) <= 0) {
            //Send NOK message
            okMessage = createMessage(NOK, NULL);
            if (writeMessage(sockW, okMessage) < 0) {
                perror("game-master : main.c : Could not send NOK message to WP\n");
                free(okMessage);
                freeBoard(board);
                free(blackName);
                free(whiteName);
                return disconnect(socket1, socket2, socketC, sockW, sockB, sockC);
            }
            free(okMessage);
            if (newMove != NULL) free(newMove);
            //Black player just lost
            hasLost = WHITE;
            exit = 1;
            continue;
        }
        ///Send OK message to WP
        okMessage = createMessage(OK, NULL);
        if (writeMessage(sockW, okMessage) < 0) {
            perror("game-master : main.c : Could not send OK message to WP\n");
            free(okMessage);
            freeBoard(board);
            free(blackName);
            free(whiteName);
            return disconnect(socket1, socket2, socketC, sockW, sockB, sockC);
        }
        free(okMessage);

        ///Update board with new move
        updateBoard(board, newMove, WHITE);
        free(newMove);

        ///Calculate scores
        scores = calculateScores(board, 1, 1); //returns two unsigned short : x white , y: black
        whiteScore = scores->x;
        blackScore = scores->y;
        free(scores);

        /////////////////////////////////////////////////
        //////////SEND NEW BOARD TO AFFICHAGE////////////
        playersData = (PlayersData*)malloc(sizeof(PlayerData));
        playersData->dataBP = (PlayerData*)malloc(sizeof(PlayerData));
        playersData->dataWP = (PlayerData*)malloc(sizeof(PlayerData));
        playersData->dataBP->playerName = blackName;
        playersData->dataWP->playerName = whiteName;
        playersData->dataBP->points = whiteScore;
        playersData->dataWP->points = blackScore;
        playersData->dataBP->timer = 1;
        playersData->dataWP->timer = 1;
        displayAllData(board, playersData);
        free(playersData->dataBP);
        free(playersData->dataWP);
        free(playersData);
        ////////////////END AFFICHAGE////////////////////
        /////////////////////////////////////////////////

        if (getNumberOfRemainingMoves(board, WHITE) <= 0) {
            exit = 1;
            continue;
        }
        if (getNumberOfRemainingMoves(board, BLACK) <=0) {
            exit = 1;
            continue;
        }
    }

    ///Send END message
    char *endMessage = createMessage(END, NULL);
    if(writeMessage(sockW, endMessage) < 0) {
        perror("game-master : main.c : Could not send END message to WP\n");
        free(endMessage);
        freeBoard(board);
        free(blackName);
        free(whiteName);
        return disconnect(socket1, socket2, socketC, sockW, sockB, sockC);
    }
    if(writeMessage(sockB, endMessage) < 0) {
        perror("game-master : main.c : Could not send END message to BP\n");
        free(endMessage);
        freeBoard(board);
        free(blackName);
        free(whiteName);
        return disconnect(socket1, socket2, socketC, sockW, sockB, sockC);
    }
    /*if(writeMessage(sockC, endMessage) < 0) {
        perror("game-master : main.c : Could not send END message to WP\n");
        free(endMessage);
        freeBoard(board);
        free(blackName);
        free(whiteName);
        return disconnect(socket1, socket2, socketC, sockW, sockB, sockC);
    }*/
    free(endMessage);

    freeBoard(board);
    free(blackName);
    free(whiteName);

    return disconnect(socket1, socket2, socketC, sockW, sockB, sockC);
}
