#include "connection.h"
#include "message.h"
#include "play.h"
#include "display.h"

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
    dataSend->playerName = name;
    char *connectMessage = createMessage(CONNECT, dataSend);
    if (writeMessage(socket, connectMessage) < 0) {
        perror("game-player : main.c : Could not send CONNECT message\n");
        return disconnect(socket);
    }
    printf("Sent CONNECT message to game-master\n");

    ///Wait for OK message
    char *okMessage = readMessage(socket);
    MessageDataRead *dataRead = (MessageDataRead*)malloc(sizeof(MessageDataRead));
    if (extractMessage(okMessage, dataRead) != INIT_OK) {
        perror("game-player : main.c : Could not read OK message\n");
        return disconnect(socket);
    }
    Color playerColor = dataRead->playerColor;
    printf("Received OK message from game-master\n");

    ///Game loop
    while (1) {
        ///Wait for NEXT_TURN message
        char *nextTurnMessage = readMessage(socket);
        MessageType result = extractMessage(nextTurnMessage, dataRead);
        if (result == NEXT_TURN) {
            Board *board = dataRead->board;
            displayBoard(board);

            ///Find the best move
            Coords *bestMove = findBestMove(board, playerColor);
            freeBoard(board);
printf("Best move found at location x=%d and y=%d\n",bestMove->x, bestMove->y);
            ///Send NEW_MOVE message
            char *messageToSend;
            dataSend->newMoveCoords = bestMove;
            messageToSend = createMessage(NEW_MOVE, dataSend);
            free(dataSend->newMoveCoords);

            if (writeMessage(socket, messageToSend) < 0) {
                perror("game-player : main.c : Could not send the new move message\n");
                return disconnect(socket);
            }
            printf("Sent NEW_MOVE message to game-master\n");

            ///Wait for OK or NOK message
            okMessage = readMessage(socket);
            result = extractMessage(okMessage, NULL);
            if (result != OK && result != NOK) {
                if (result == END) printf("game-player : Received END message: disconnecting...\n");
                else perror("game-player : main.c : Could not read OK or NOK message\n");
                return disconnect(socket);
            }
            printf("Received %s message from game-master\n",result == OK ? "OK" : "NOK");
        }
        else {
            if (result == END) printf("game-player : Received END message: disconnecting...\n");
            else printf("game-player : main.c : Expected message of type NEXT_TURN\n");
            return disconnect(socket);
        }
    }

    int ret = disconnect(socket);
    printf("game-player stopping\n");
    return ret;
}
