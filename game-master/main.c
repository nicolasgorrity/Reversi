#include "connection.h"
#include "message.h"
#include "play.h"
#include "display.h"

void clean_stdin(void)
{
    int c;
    do {
        c = getchar();
    } while (c != '\n' && c != EOF);
}

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

    int socket1 = createSocket();
    if (bindSocket(socket1, portPlayer1) < 0) return disconnect(socket1, -1, -1, -1);
    if (listenSocket(socket1) < 0) return disconnect(socket1, -1, -1, -1);

    int sockB = acceptSocket(socket1);
    if (sockB < 0) return disconnect(socket1, -1, -1, -1);
    printf("First player connected\n");

    int sockW = acceptSocket(socket1);
    if (sockW < 0) return disconnect(socket1, sockB, -1, -1);
    printf("Second player connected\n");

    int sockC = -1;
    /*if (listenSocket(socketC) < 0) return disconnect(socket1, sockW, sockB, -1);;
    int sockC = acceptSocket(socketC);
    if (sockC < 0) return disconnect(socket1, sockW, sockB, -1);;
*/
    ////////Black player///////
    ///Wait for Connect message
    char *connectMsgB = readMessage(sockB);
    MessageDataRead *dataRead = (MessageDataRead*)malloc(sizeof(MessageDataRead));
    if (extractMessage(connectMsgB, dataRead) != CONNECT) {
        perror("game-master : main.c : Could not read CONNECT message\n");
        free(connectMsgB);
        free(dataRead);
        return disconnect(socket1, sockW, sockB, sockC);
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
        return disconnect(socket1, sockW, sockB, sockC);
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
        return disconnect(socket1, sockW, sockB, sockC);
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
        return disconnect(socket1, sockW, sockB, sockC);
    }
    printf("Sent OK message to WP\n");
    freeString(okMessage);

    ///////////////////////////////////////////
    ////////Message to Game Controller????/////
    ///////////////////////////////////////////

    char endOfGame = 0;
    while (!endOfGame) {

        Board *board = allocateInitialBoard(8, 8);
        char whiteCannotPlay=0;
        char blackCannotPlay=0;

        unsigned short whiteScore;
        unsigned short blackScore;

        ///Game Loop
        char exit = 0;
        Color hasLost = EMPTY;
        while (!exit) {
            if (!blackCannotPlay) {
                ///Ask and receive new move from BP:
                printf("Requesting new move to BP...\n");
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
                        return disconnect(socket1, sockW, sockB, sockC);
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
                    return disconnect(socket1, sockW, sockB, sockC);
                }
                printf("Sent OK message to BP\n");
                freeString(okMessage);

                ///Update board with new move
                updateBoard(board, newMove, BLACK);
                free(newMove);

                ///Calculate scores
                Coords *scores = calculateScores(board, 1, 1); //returns two unsigned short : x white , y: black
                whiteScore = scores->x;
                blackScore = scores->y;
                free(scores);

                /////////////////////////////////////////////////
                //////////SEND NEW BOARD TO AFFICHAGE////////////
                displayBoard(board);
                printf("White score: %d\n",whiteScore);
                printf("Black score: %d\n",blackScore);
                ////////////////END AFFICHAGE////////////////////
                /////////////////////////////////////////////////
            }

            //If the opponent has no more possibilities to play, that's the end
            if (getNumberOfRemainingMoves(board, WHITE) <=0) {
                whiteCannotPlay = 1;
                if (blackCannotPlay == 1) {
                    exit = 1;
                    continue;
                }
            }
            else {
                whiteCannotPlay = 0;
            }

            if (!whiteCannotPlay) {
                ///Ask and receive new move from WP:
                printf("Requesting new move to WP...\n");
                Coords *newMove = requestPlayerMove(sockW, board);
                while (newMove == NULL || isMoveValid(board, newMove, WHITE) <= 0) {
                    //Send NOK message
                    okMessage = createMessage(NOK, NULL);
                    if (writeMessage(sockW, okMessage) < 0) {
                        perror("game-master : main.c : Could not send NOK message to WP\n");
                        freeString(okMessage);
                        freeBoard(board);
                        freeString(blackName);
                        freeString(whiteName);
                        return disconnect(socket1, sockW, sockB, sockC);
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
                    return disconnect(socket1, sockW, sockB, sockC);
                }
                printf("Sent OK message to WP\n");
                freeString(okMessage);

                ///Update board with new move
                updateBoard(board, newMove, WHITE);
                free(newMove);

                ///Calculate scores
                Coords *scores = calculateScores(board, 1, 1); //returns two unsigned short : x white , y: black
                whiteScore = scores->x;
                blackScore = scores->y;
                free(scores);

                /////////////////////////////////////////////////
                //////////SEND NEW BOARD TO AFFICHAGE////////////
                displayBoard(board);
                printf("White score: %d\n",whiteScore);
                printf("Black score: %d\n",blackScore);
                ////////////////END AFFICHAGE////////////////////
                /////////////////////////////////////////////////
            }

            //If the opponent has no more possibilities to play, that's the end
            if (getNumberOfRemainingMoves(board, BLACK) <=0) {
                blackCannotPlay = 1;
                if (whiteCannotPlay == 1) {
                    exit = 1;
                    continue;
                }
            }
            else {
                blackCannotPlay = 0;
            }
        }
        ///End of this game
        printf("--- END OF GAME ---\n");
        displayBoard(board);
        printf("White score: %d\n",whiteScore);
        printf("Black score: %d\n",blackScore);
        freeBoard(board);

        /////////////////////////NEW LEVEL ?///////////////////
        printf("\nNew game ?\n");
        char c = getchar();
        clean_stdin();
        printf("%c\n",c);
        if (c!='y' && c!='Y' && c!='o' && c!='O') {
            endOfGame = 1;
            continue;
        }
        ///////////////////////////////////////////////////////
    }

    ///Send END message
    String *endMessage = createMessage(END, NULL);
    if(writeMessage(sockW, endMessage) < 0) {
        perror("game-master : main.c : Could not send END message to WP\n");
        freeString(endMessage);
        freeString(blackName);
        freeString(whiteName);
        return disconnect(socket1, sockW, sockB, sockC);
    }
    printf("Sent END message to WP\n");
    if(writeMessage(sockB, endMessage) < 0) {
        perror("game-master : main.c : Could not send END message to BP\n");
        freeString(endMessage);
        freeString(blackName);
        freeString(whiteName);
        return disconnect(socket1, sockW, sockB, sockC);
    }
    printf("Sent END message to BP\n");
    /*if(writeMessage(sockC, endMessage) < 0) {
        perror("game-master : main.c : Could not send END message to WP\n");
        free(endMessage);
        freeBoard(board);
        free(blackName);
        free(whiteName);
        return disconnect(socket1, sockW, sockB, sockC);
    }*/
    freeString(endMessage);

    freeString(blackName);
    freeString(whiteName);

    return disconnect(socket1, sockW, sockB, sockC);
}
