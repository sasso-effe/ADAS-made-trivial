#include "socketCommunication.h"
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <unistd.h>

void sendToSocket(int socketFd, const char* data){
	write(socketFd, data, strlen(data)+1);
}

int readFromSocket (int fd, char *str) {
	int n;
	do {
		n = read (fd, str, 1);
	} while (n > 0 && *str++ != '\0');
return (n > 0);
}

void readFromClient(int socketFd){
	char str[200];
	while (readFromSocket (socketFd, str)) /* Read lines until end-of-input */
	printf ("Ricevuto: %s\n", str); /* Echo line from socket */
}

/* 	---- HOW TO USE readFromSocket ---

	char str[MAX_RESPONSE_SIZE]; --- where the data will be stored
	while (readFromSocket (socketFd, str)) --- loop on reading
	printf ("Ricevuto: %s\n", str); --- use readed data

}*/

int createConnection(char *socketName){
	int socketFd, serverLen;
	struct sockaddr_un serverUNIXAddress;
	struct sockaddr* serverSockAddrPtr;
	serverSockAddrPtr = (struct sockaddr*) &serverUNIXAddress;
	serverLen = sizeof (serverUNIXAddress);
	socketFd = socket (AF_UNIX, SOCK_STREAM, DEFAULT_PROTOCOL);
	serverUNIXAddress.sun_family = AF_UNIX; /* Server domain */
	strcpy (serverUNIXAddress.sun_path, socketName);/*Server name*/
	int result = connect(socketFd, serverSockAddrPtr, serverLen);
 	if(result < 0){
 		return result;
 	}
 	return socketFd;
}
