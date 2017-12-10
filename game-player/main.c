#include "connection.h"
#include "message.h"
#include "artificial_intelligence.h"
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
            displayBoard(board);

            ///Find the best move
            Coords *bestMove = findBestMove(board, playerColor);
            freeBoard(board);
            printf("Best move found at location x=%d and y=%d\n",bestMove->x, bestMove->y);

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
            if (result == END) printf("game-player : Received END message: disconnecting...\n");
            else printf("game-player : main.c : Expected message of type NEXT_TURN\n");
            return disconnect(socket);
        }
    }

    return disconnect(socket);
}
