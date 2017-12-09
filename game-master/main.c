#include "connection.h"
#include "message.h"
#include "play.h"
#include "display.h"

Coords* requestPlayerMove(int playerSocket, Board *board) {
    ///Send NEXT-TURN message to player
    MessageDataSend *dataSend = (MessageDataSend*)malloc(sizeof(MessageDataSend));
    dataSend->board = board;
    String *nextTurnMsg = createMessage(NEXT_TURN, dataSend);
    free(dataSend);
    if(writeMessage(playerSocket, nextTurnMsg) < 0) {
        perror("game-master : main.c : requestPlayerMove() : Could not send NEXT_TURN message\n");
        freeString(nextTurnMsg);
        return NULL;
    }
    printf("Sent NEXT_TURN message to game-player\n");
    freeString(nextTurnMsg);

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
    printf("Received NEW_MOVE message from game-player\n");

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
    int socket2 = -1;//createSocket();
    int socketC = -1;//createSocket();

    if (bindSocket(socket1, portPlayer1) < 0) return disconnect(socket1, socket2, socketC, -1, -1, -1);
    //if (bindSocket(socket2, portPlayer2) < 0) return disconnect(socket1, socket2, socketC, -1, -1, -1);
    //if (bindSocket(socketC, portController) < 0) return disconnect(socket1, socket2, socketC, -1, -1, -1);

    if (listenSocket(socket1) < 0) return disconnect(socket1, socket2, socketC, -1, -1, -1);

    int sockB = acceptSocket(socket1);
    if (sockB < 0) return disconnect(socket1, socket2, socketC, -1, -1, -1);
    printf("First player connected\n");

    //if (listenSocket(socket2) < 0) return disconnect(socket1, socket2, socketC, sockB, -1, -1);
    int sockW = acceptSocket(socket1);
    if (sockW < 0) return disconnect(socket1, socket2, socketC, sockB, -1, -1);
    printf("Second player connected\n");

    int sockC = -1;
    /*if (listenSocket(socketC) < 0) return disconnect(socket1, socket2, socketC, sockW, sockB, -1);;
    int sockC = acceptSocket(socketC);
    if (sockC < 0) return disconnect(socket1, socket2, socketC, sockW, sockB, -1);;
*/
    ////////Black player///////
    ///Wait for Connect message
    char *connectMsgB = readMessage(sockB);
    MessageDataRead *dataRead = (MessageDataRead*)malloc(sizeof(MessageDataRead));
    if (extractMessage(connectMsgB, dataRead) != CONNECT) {
        perror("game-master : main.c : Could not read CONNECT message\n");
        free(connectMsgB);
        free(dataRead);
        return disconnect(socket1, socket2, socketC, sockW, sockB, sockC);
    }
    printf("Received CONNECT message from BP\n");
    String* blackName = dataRead->playerName;
    free(dataRead);
    free(connectMsgB);

    ///Send OK message
    MessageDataSend *dataSend = (MessageDataSend*)malloc(sizeof(MessageDataSend));
    dataSend->playerColor = BLACK;
    String *okMessage = createMessage(INIT_OK, dataSend);
    free(dataSend);
    if(writeMessage(sockB, okMessage) < 0) {
        perror("game-master : main.c : Could not send OK message\n");
        freeString(blackName);
        freeString(okMessage);
        return disconnect(socket1, socket2, socketC, sockW, sockB, sockC);
    }
    printf("Sent OK message to BP\n");
    freeString(okMessage);

    ////////White player///////
    ///Wait for Connect message
    char *connectMsgW = readMessage(sockW);
    dataRead = (MessageDataRead*)malloc(sizeof(MessageDataRead));
    if (extractMessage(connectMsgW, dataRead) != CONNECT) {
        perror("game-master : main.c : Could not read CONNECT message\n");
        freeString(blackName);
        free(connectMsgW);
        free(dataRead);
        return disconnect(socket1, socket2, socketC, sockW, sockB, sockC);
    }
    String *whiteName = dataRead->playerName;
    printf("Received CONNECT message from WP\n");
    free(dataRead);
    free(connectMsgW);

    ///Send OK message
    dataSend = (MessageDataSend*)malloc(sizeof(MessageDataSend));
    dataSend->playerColor = WHITE;
    okMessage = createMessage(INIT_OK, dataSend);
    free(dataSend);
    if(writeMessage(sockW, okMessage) < 0) {
        perror("game-master : main.c : Could not send INIT_OK message\n");
        freeString(blackName);
        freeString(whiteName);
        freeString(okMessage);
        return disconnect(socket1, socket2, socketC, sockW, sockB, sockC);
    }
    printf("Sent OK message to WP\n");
    freeString(okMessage);

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
                freeString(okMessage);
                freeBoard(board);
                freeString(blackName);
                freeString(whiteName);
                return disconnect(socket1, socket2, socketC, sockW, sockB, sockC);
            }
            printf("Sent NOK message to BP\n");
            freeString(okMessage);
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
            freeString(okMessage);
            freeBoard(board);
            freeString(blackName);
            freeString(whiteName);
            return disconnect(socket1, socket2, socketC, sockW, sockB, sockC);
        }
        printf("Sent OK message to BP\n");
        freeString(okMessage);

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
        displayBoard(board);
        ////////////////END AFFICHAGE////////////////////
        /////////////////////////////////////////////////

        ///Ask and receive new move from WP:
        newMove = requestPlayerMove(sockW, board);
        while (newMove == NULL || isMoveValid(board, newMove, WHITE) <= 0) {
            //Send NOK message
            okMessage = createMessage(NOK, NULL);
            if (writeMessage(sockW, okMessage) < 0) {
                perror("game-master : main.c : Could not send NOK message to WP\n");
                freeString(okMessage);
                freeBoard(board);
                freeString(blackName);
                freeString(whiteName);
                return disconnect(socket1, socket2, socketC, sockW, sockB, sockC);
            }
            printf("Sent NOK message to WP\n");
            freeString(okMessage);
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
            freeString(okMessage);
            freeBoard(board);
            freeString(blackName);
            freeString(whiteName);
            return disconnect(socket1, socket2, socketC, sockW, sockB, sockC);
        }
        printf("Sent OK message to WP\n");
        freeString(okMessage);

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
        displayBoard(board);
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
    String *endMessage = createMessage(END, NULL);
    if(writeMessage(sockW, endMessage) < 0) {
        perror("game-master : main.c : Could not send END message to WP\n");
        freeString(endMessage);
        freeBoard(board);
        freeString(blackName);
        freeString(whiteName);
        return disconnect(socket1, socket2, socketC, sockW, sockB, sockC);
    }
    printf("Sent END message to WP\n");
    if(writeMessage(sockB, endMessage) < 0) {
        perror("game-master : main.c : Could not send END message to BP\n");
        freeString(endMessage);
        freeBoard(board);
        freeString(blackName);
        freeString(whiteName);
        return disconnect(socket1, socket2, socketC, sockW, sockB, sockC);
    }
    printf("Sent END message to BP\n");
    /*if(writeMessage(sockC, endMessage) < 0) {
        perror("game-master : main.c : Could not send END message to WP\n");
        free(endMessage);
        freeBoard(board);
        free(blackName);
        free(whiteName);
        return disconnect(socket1, socket2, socketC, sockW, sockB, sockC);
    }*/
    freeString(endMessage);

    freeBoard(board);
    freeString(blackName);
    freeString(whiteName);

    return disconnect(socket1, socket2, socketC, sockW, sockB, sockC);
}
