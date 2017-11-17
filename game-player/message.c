#include "message.h"

char *createMessage(Message message) {
    char *mess = NULL;

    switch (message) {
    case OK:
        char type = 0x02;
        char okCode = 0x01;
        //Create the string to return
        mess = (char*)malloc(length * 3));
        //Fill it
        mess[0] = type;
        mess[1] = okCode;
        mess[2] = '\0';
    }

    return mess;
}
