#include "connection.h"
#include "message.h"
#include "play.h"


int main(int argc, char *argv[])
{
    unsigned int portPlayer1 = 8888;
    unsigned int portPlayer2 = 8889;
    unsigned int portController = 8890;


    int socket = createSocket();
    if (bindSocket(socket, portPlayer1) < 0) return disconnect(socket, -1, -1, -1);
    if (bindSocket(socket, portPlayer2) < 0) return disconnect(socket, -1, -1, -1);
    if (bindSocket(socket, portController) < 0) return disconnect(socket, -1, -1, -1);

    if (listenSocket(socket) < 0) return disconnect(socket, -1, -1, -1);
    int sock1 = acceptSocket(socket);
    if (sock1 < 0) return disconnect(socket, -1, -1, -1);

    if (listenSocket(socket) < 0) return disconnect(socket, sock1, -1, -1);
    int sock2 = acceptSocket(socket);
    if (sock2 < 0) return disconnect(socket, sock1, -1, -1);;

    if (listenSocket(socket) < 0) return disconnect(socket, sock1, sock2, -1);;
    int sock3 = acceptSocket(socket);
    if (sock3 < 0) return disconnect(socket, sock1, sock2, -1);;

    ////////White player///////
    ///Wait for Connect message
    char *connectMsgW = readMessage(sock1);
    MessageDataRead *dataRead = (MessageDataRead*)malloc(sizeof(MessageDataRead));
    if (extractMessage(connectMsgW, dataRead) != CONNECT) {
        perror("game-master : main.c : Could not read CONNECT message\n");
        return disconnect(socket, sock1, sock2, sock3);
    }
    char* whiteName = dataRead->playerName;
    ///Send OK message
    MessageDataSend *dataSend = (MessageDataSend*)malloc(sizeof(MessageDataSend));
    dataSend->playerColor = WHITE;
    char *okMessage = createMessage(INIT_OK, dataSend);
    if(writeMessage(sock1, okMessage) < 0) {
        perror("game-master : main.c : Could not send OK message\n");
        return disconnect(socket, sock1, sock2, sock3);
    }

    ////////Black player///////
    ///Wait for Connect message
    char *connectMsgB = readMessage(sock2);
    if (extractMessage(connectMsgB, dataRead) != CONNECT) {
        perror("game-master : main.c : Could not read CONNECT message\n");
        return disconnect(socket, sock1, sock2, sock3);
    }
    char* blackName = dataRead->playerName;
    ///Send OK message
    dataSend->playerColor = BLACK;
    okMessage = createMessage(INIT_OK, dataSend);
    if(writeMessage(sock2, okMessage) < 0) {
        perror("game-master : main.c : Could not send OK message\n");
        return disconnect(socket, sock1, sock2, sock3);
    }



    /////////////////////////////
    ////////SockConnect ????/////
    /////////////////////////////




    Board *board = allocateInitialBoard(8, 8);

    ///Game Loop
    while(1) {
        ///Send NEXT-TURN message to BP
        dataSend->board = board;
        char *nextTurnMsgB = createMessage(NEXT_TURN, dataSend);
        if(writeMessage(sock2, nextTurnMsgB) < 0) {
            perror("game-master : main.c : Could not send NEXT_TURN message\n");
            return disconnect(socket, sock1, sock2, sock3);
        }

        ///Wait for NEW_MOVE message
        char *newMoveMsgB = readMessage(sock2);
        if (extractMessage(newMoveMsgB, dataRead) != NEW_MOVE) {
            perror("game-master : main.c : Could not read NEW_MOVE message\n");
        }
        Coords *newMove = dataRead->newMoveCoords;
    }


    return 0;
}
