#include <pthread.h>
#include <time.h>
#include "connection.h"
#include "message.h"
#include "play.h"
#include "display.h"

#define GC_ACTIVE 0 //1 to enable compatibility with Game Controller

int socket1;
int sockB;
int sockW;
int sockC;

void clean_stdin(void)
{
    int c;
    do {
        c = getchar();
    } while (c != '\n' && c != EOF);
}

void *pingThreadFunc(void *ptSocket) {
    int socket = *((int *) ptSocket);
    printf("coucou");
    free(ptSocket);
    printf("coucou");
    clock_t timeBegin = clock();
    clock_t lastOKtime = timeBegin;
    while (1) {
        clock_t now = clock();
        if ((double)(now - timeBegin) > 10.0*CLOCKS_PER_SEC) {
            ///Player took too much time
            perror("The client took too much time to respond.\nEnding the program...\n");
            disconnect(socket1, sockB, sockW, sockC);
            exit(-1);
            return NULL;
        }
        if ((double)(now - lastOKtime) > 1.0*CLOCKS_PER_SEC) {
            ///Send PING message
            String *pingMessage = createMessage(PING, NULL);
            if (writeMessage(socket, pingMessage) < 0) {
                perror("game-master : main.c : pingThreadFunc : Could not send PING message\n");
                freeString(pingMessage);
                return NULL;
            }
            freeString(pingMessage);

            ///Wait OK message
            char *okMessage = readMessage(socket);
            if (extractMessage(okMessage, NULL) != OK) {
                perror("game-master : main.c: pingThreadFunc : Did not receive OK message after PING\n");
                free(okMessage);
            } else {
                lastOKtime = clock();
                free(okMessage);
            }
        }
    }
}

Coords* requestPlayerMove(int playerSocket, Board *board, unsigned short *timeTaken) {
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

    //While waiting, launch a thread for PING
    /*pthread_t pthread1;
    int *i = (int*)malloc(sizeof(int));
    i[0]=playerSocket;
    printf("Coucou");
    if (pthread_create(&pthread1, NULL, pingThreadFunc, (void*)i) != 0) perror("Could not create thread for PING feature\n");
*/
    clock_t begin = clock();

    ///Wait for NEW_MOVE message
    char *newMoveMsg = readMessage(playerSocket);
    clock_t end = clock();
    *timeTaken = (end - begin) / CLOCKS_PER_SEC;
    //pthread_cancel(pthread1);
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

    socket1 = createSocket();
    if (bindSocket(socket1, portPlayer1) < 0) return disconnect(socket1, -1, -1, -1);
    if (listenSocket(socket1) < 0) return disconnect(socket1, -1, -1, -1);

    sockB = acceptSocket(socket1);
    if (sockB < 0) return disconnect(socket1, -1, -1, -1);
    printf("First player connected\n");

    sockW = acceptSocket(socket1);
    if (sockW < 0) return disconnect(socket1, sockB, -1, -1);
    printf("Second player connected\n");

    sockC = -1;
    printf("Waiting for game-controller...\n");
    if (GC_ACTIVE) {
        int sockC = acceptSocket(socket1);
        if (sockC < 0) return disconnect(socket1, sockW, sockB, -1);
        printf("Game controller connected\n");
    }

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

    PlayersData *playersData = (PlayersData*)malloc(sizeof(PlayersData));
    playersData->dataBP = (PlayerData*)malloc(sizeof(PlayerData));
    playersData->dataWP = (PlayerData*)malloc(sizeof(PlayerData));
    playersData->dataBP->playerName = blackName;
    playersData->dataWP->playerName = whiteName;

    char start = 0;
    char endOfGame = 0;
    unsigned short sizeX=8, sizeY=8;
    char stepByStep = 0;

    while (!endOfGame) {
        start=0;
        ///Wait for CONTROL message saying to start from GC
        while (GC_ACTIVE && !start) {
            char *controlMsg = readMessage(sockC);
            MessageDataRead *dataRead = (MessageDataRead*)malloc(sizeof(MessageDataRead));
            if (extractMessage(controlMsg, dataRead) != CONTROL) {
                perror("game-master : main.c : Could not read CONTROL message from GC\n");
                free(dataRead);
                free(controlMsg);
                return disconnect(socket1, sockB, sockW, sockC);
            }
            start = dataRead->control->restart;
            free(controlMsg);
            sizeX = dataRead->control->newBoardSize->x;
            sizeY = dataRead->control->newBoardSize->y;
            stepByStep = dataRead->control->mode;
            free(dataRead->control->newBoardSize);
            free(dataRead->control);
            free(dataRead);
            printf("Received CONTROL message from game-player\n");
        }
        start=0;

        Board *board = allocateInitialBoard(sizeX, sizeY);
        char whiteCannotPlay=0;
        char blackCannotPlay=0;

        playersData->dataBP->points = 0;
        playersData->dataWP->points = 0;
        playersData->dataBP->timer = 0;
        playersData->dataWP->timer = 0;

        if (GC_ACTIVE) {
            ///Send Status messages to the board
            dataSend = (MessageDataSend*)malloc(sizeof(MessageDataSend));
            dataSend->board = board;
            String *statusMsg = createMessage(STATUS1, dataSend);
            if (writeMessage(sockC, statusMsg) < 0) {
                perror("Could not send STATUS1 message to game-controller\n");
                freeString(statusMsg);
                freePlayersData(playersData);
                freeBoard(board);
                return disconnect(socket1, sockW, sockB, sockC);
            }
            freeString(statusMsg);
            printf("Sent STATUS1 message to Game controller\n");

            dataSend->playersData = playersData;
            statusMsg = createMessage(STATUS2, dataSend);
            if (writeMessage(sockC, statusMsg) < 0) {
                perror("Could not send STATUS2 message to game-controller\n");
                freeString(statusMsg);
                freePlayersData(playersData);
                freeBoard(board);
                return disconnect(socket1, sockW, sockB, sockC);
            }
            freeString(statusMsg);
            printf("Sent STATUS2 message to Game controller\n");
        }
        else displayBoard(board);

        ///Game Loop
        char exit = 0;
        Color hasLost = EMPTY;
        while (!exit) {
            ///STEP BY STEP OR AUTONOMOUS
            if (stepByStep && GC_ACTIVE) {
                char *controlMsg = readMessage(sockC);
                MessageDataRead *dataRead = (MessageDataRead*)malloc(sizeof(MessageDataRead));
                if (extractMessage(controlMsg, dataRead) != CONTROL) {
                    perror("game-master : main.c : Could not read CONTROL message from GC\n");
                    free(dataRead);
                    free(controlMsg);
                    return disconnect(socket1, sockB, sockW, sockC);
                }
                printf("Received CONTROL message from game-player\n");
                start = dataRead->control->restart;
                sizeX = dataRead->control->newBoardSize->x;
                sizeY = dataRead->control->newBoardSize->y;
                stepByStep = dataRead->control->mode;
                free(dataRead->control->newBoardSize);
                free(dataRead->control);
                free(dataRead);
                free(controlMsg);
                if (start) {
                    exit=1;
                    continue;
                }
            }

            if (!blackCannotPlay) {
                ///Ask and receive new move from BP:
                printf("Requesting new move to BP...\n");
                unsigned short timeTaken;
                Coords *newMove = requestPlayerMove(sockB, board, &timeTaken);
                if (newMove == NULL || isMoveValid(board, newMove, BLACK) <= 0) {
                    //Send NOK message
                    okMessage = createMessage(NOK, NULL);
                    if (writeMessage(sockB, okMessage) < 0) {
                        perror("game-master : main.c : Could not send NOK message to BP\n");
                        freeString(okMessage);
                        freeBoard(board);
                        freePlayersData(playersData);
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
                    freePlayersData(playersData);
                    return disconnect(socket1, sockW, sockB, sockC);
                }
                printf("Sent OK message to BP\n");
                freeString(okMessage);

                ///Update board with new move
                updateBoard(board, newMove, BLACK);
                free(newMove);

                ///Calculate scores
                Coords *scores = calculateScores(board, 1, 1); //returns two unsigned short : x white , y: black
                playersData->dataWP->points = scores->x;
                playersData->dataBP->points = scores->y;
                free(scores);

                ///Update BP timer
                playersData->dataBP->timer += timeTaken;

                ///Display the updated board, scores and timers
                if (GC_ACTIVE) {
                    //Send Status messages to the board
                    dataSend = (MessageDataSend*)malloc(sizeof(MessageDataSend));
                    dataSend->board = board;
                    String *statusMsg = createMessage(STATUS1, dataSend);
                    if (writeMessage(sockC, statusMsg) < 0) {
                        perror("Could not send STATUS1 message to game-controller\n");
                        freeString(statusMsg);
                        freePlayersData(playersData);
                        freeBoard(board);
                        return disconnect(socket1, sockW, sockB, sockC);
                    }
                    freeString(statusMsg);
                    printf("Sent STATUS1 message to Game controller\n");

                    dataSend->playersData = playersData;
                    statusMsg = createMessage(STATUS2, dataSend);
                    if (writeMessage(sockC, statusMsg) < 0) {
                        perror("Could not send STATUS2 message to game-controller\n");
                        freeString(statusMsg);
                        freePlayersData(playersData);
                        freeBoard(board);
                        return disconnect(socket1, sockW, sockB, sockC);
                    }
                    freeString(statusMsg);
                    printf("Sent STATUS2 message to Game controller\n");
                }
                else {
                    displayAllData(board, playersData);
                }
            }

            //If the opponent has no more possibilities to play, that's the end
            if (getNumberOfRemainingMoves(board, WHITE) <=0) {
                whiteCannotPlay = 1;
                printf("WP has no possibility!\n");
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
                unsigned short timeTaken;
                Coords *newMove = requestPlayerMove(sockW, board, &timeTaken);
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
                playersData->dataWP->points = scores->x;
                playersData->dataBP->points = scores->y;
                free(scores);

                ///Update WP timer
                playersData->dataWP->timer += timeTaken;

                ///Display the updated board, scores and timers
                if (GC_ACTIVE) {
                    //Send Status messages to the board
                    dataSend = (MessageDataSend*)malloc(sizeof(MessageDataSend));
                    dataSend->board = board;
                    String *statusMsg = createMessage(STATUS1, dataSend);
                    if (writeMessage(sockC, statusMsg) < 0) {
                        perror("Could not send STATUS1 message to game-controller\n");
                        freeString(statusMsg);
                        freePlayersData(playersData);
                        freeBoard(board);
                        return disconnect(socket1, sockW, sockB, sockC);
                    }
                    freeString(statusMsg);
                    printf("Sent STATUS1 message to Game controller\n");

                    dataSend->playersData = playersData;
                    statusMsg = createMessage(STATUS2, dataSend);
                    if (writeMessage(sockC, statusMsg) < 0) {
                        perror("Could not send STATUS2 message to game-controller\n");
                        freeString(statusMsg);
                        freePlayersData(playersData);
                        freeBoard(board);
                        return disconnect(socket1, sockW, sockB, sockC);
                    }
                    freeString(statusMsg);
                    printf("Sent STATUS2 message to Game controller\n");
                }
                else {
                    displayAllData(board, playersData);
                }
            }

            //If the opponent has no more possibilities to play, that's the end
            if (getNumberOfRemainingMoves(board, BLACK) <=0) {
                blackCannotPlay = 1;
                printf("BP has no possibility!\n");
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
        printf("\n--- END OF GAME ---\n");
        displayBoard(board);
        if (hasLost == BLACK) playersData->dataWP->points = 0;
        else if (hasLost == WHITE) playersData->dataBP->points = 0;
        printf("White score: %d\n",playersData->dataWP->points);
        printf("Black score: %d\n",playersData->dataBP->points);
        freeBoard(board);

        if (GC_ACTIVE) {
            if (start==0) endOfGame = 1;
        }
        else {
            printf("\nNew game ?\n");
            char c = getchar();
            clean_stdin();
            if (c!='y' && c!='Y' && c!='o' && c!='O') {
                endOfGame = 1;
                continue;
            }
        }
    }

    ///Send END message
    String *endMessage = createMessage(END, NULL);
    if(writeMessage(sockW, endMessage) < 0) {
        perror("game-master : main.c : Could not send END message to WP\n");
        freeString(endMessage);
        freePlayersData(playersData);
        return disconnect(socket1, sockW, sockB, sockC);
    }
    printf("Sent END message to WP\n");
    if(writeMessage(sockB, endMessage) < 0) {
        perror("game-master : main.c : Could not send END message to BP\n");
        freeString(endMessage);
        freePlayersData(playersData);
        return disconnect(socket1, sockW, sockB, sockC);
    }
    printf("Sent END message to BP\n");
    if (GC_ACTIVE) {
        if (writeMessage(sockC, endMessage) < 0) {
            perror("game-master : main.c : Could not send END message to WP\n");
            freeString(endMessage);
            freePlayersData(playersData);
            return disconnect(socket1, sockW, sockB, sockC);
        }
    }

    freeString(endMessage);

    freePlayersData(playersData);

    return disconnect(socket1, sockW, sockB, sockC);
}
