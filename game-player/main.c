#include "connection.h"
#include "message.h"
#include "artificial_intelligence.h"
#include "display.h"
#include <pthread.h>

void *pingThreadFunc(void *ptSocket) {
    int socket = *((int *) ptSocket);
    free(ptSocket);
    while (1) {
        ///Wait for PING message
        char *pingMessage = readMessage(socket);
        if (extractMessage(pingMessage, NULL) == PING) {
            printf("Received PING message from game-master\n");
            free(pingMessage);
            ///Send OK message
            String *okMessage = createMessage(OK, NULL);
            if (writeMessage(socket, okMessage) < 0) {
                perror("game-master : main.c : pingThreadFunc : Could not send OK message after PING\n");
                freeString(okMessage);
            }
            freeString(okMessage);
            printf("Sent OK message to game-master after the PING\n");
        }
        free(pingMessage);
    }
}

int main(int argc, char *argv[])
{
    unsigned int port = 8888;
    printf("Port number: %d\n", port);
    char *name = "Jean-Mi";

    ///Open port as Client
    int socket = createSocket();
    if (connectSocket(socket, port) < 0) return -1;

    ///Send Connect message
    MessageDataSend *dataSend = (MessageDataSend*)malloc(sizeof(MessageDataSend));
    dataSend->playerName = newString(name, 7);
    String *connectMessage = createMessage(CONNECT, dataSend);
    free(dataSend);
    if (writeMessage(socket, connectMessage) < 0) {
        perror("game-player : main.c : Could not send CONNECT message\n");
        freeString(connectMessage);
        return disconnect(socket);
    }
    freeString(connectMessage);
    printf("Sent CONNECT message to game-master\n");

    ///Wait for OK message
    char *okMessage = readMessage(socket);
    MessageDataRead *dataRead = (MessageDataRead*)malloc(sizeof(MessageDataRead));
    if (extractMessage(okMessage, dataRead) != INIT_OK) {
        perror("game-player : main.c : Could not read OK message\n");
        free(okMessage);
        return disconnect(socket);
    }
    free(okMessage);
    Color playerColor = dataRead->playerColor;
    free(dataRead);
    printf("Received OK message from game-master\n");

    ///Game loop
    while (1) {
        ///Wait for NEXT_TURN message
        char *nextTurnMessage = readMessage(socket);
        dataRead = (MessageDataRead*)malloc(sizeof(MessageDataRead));
        MessageType result = extractMessage(nextTurnMessage, dataRead);
        free(nextTurnMessage);
        if (result == NEXT_TURN) {
            Board *board = dataRead->board;
            free(dataRead);
            //displayBoard(board);
            /*///While calculating, launch a thread for PING
            pthread_t pthread1;
            int *i = (int*)malloc(sizeof(int)); (*i)=socket;
            if (pthread_create(&pthread1, NULL, pingThreadFunc, (void*)i) != 0) perror("Could not create thread for PING feature\n");
*/
            ///Find the best move
            Coords *bestMove = findBestMove(board, playerColor);
            freeBoard(board);
            printf("Best move found at location x=%d and y=%d\n",bestMove->x, bestMove->y);

            ///End the thread for PING
  //          pthread_cancel(pthread1);

            ///Send NEW_MOVE message
            dataSend = (MessageDataSend*)malloc(sizeof(MessageDataSend));
            dataSend->newMoveCoords = bestMove;
            String *messageToSend = createMessage(NEW_MOVE, dataSend);
            free(dataSend->newMoveCoords);
            free(dataSend);
            if (writeMessage(socket, messageToSend) < 0) {
                perror("game-player : main.c : Could not send the new move message\n");
                freeString(messageToSend);
                return disconnect(socket);
            }
            freeString(messageToSend);
            printf("Sent NEW_MOVE message to game-master\n");

            ///Wait for OK or NOK message
            okMessage = readMessage(socket);
            result = extractMessage(okMessage, NULL);
            free(okMessage);
            if (result != OK && result != NOK) {
                if (result == END) printf("game-player : Received END message: disconnecting...\n");
                else perror("game-player : main.c : Could not read OK or NOK message\n");
                return disconnect(socket);
            }
            printf("Received %s message from game-master\n",result == OK ? "OK" : "NOK");
        }
        else {
            free(dataRead);
            if (result == PING) {
                ///If PING was received, send back a OK message
                String *messageToSend = createMessage(OK, NULL);
                if (writeMessage(socket, messageToSend) < 0) {
                    perror("game-player : main.c : Could not send the OK message after PING\n");
                    freeString(messageToSend);
                }
                freeString(messageToSend);
                printf("Sent OK message to game-master after the PING\n");
            }
            if (result == END) printf("game-player : Received END message: disconnecting...\n");
            else printf("game-player : main.c : Received unexpected message\n");
            return disconnect(socket);
        }
    }

    return disconnect(socket);
}
