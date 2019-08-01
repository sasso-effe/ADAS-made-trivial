#ifndef _SOCKETCOMMUNICATION_H_
#define _SOCKETCOMMUNICATION_H_
#define DEFAULT_PROTOCOL 0

extern void sendToSocket(int socketFd, const char* data);
extern int readFromSocket(int socketFd, char* str);
extern void readFromClient(int socketFd);
extern int createConnection(char* socketName);
#endif

