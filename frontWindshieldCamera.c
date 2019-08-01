#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h> /* For AF_UNIX sockets */
#include "socketCommunication.h"
#include "fileManagement.h"

#define DEFAULT_PROTOCOL 0
#define RECORD 1

FILE *fileToRead;
FILE *fileLog;

int socketFd;
long readPosition;

void startSensor();
char* readLine(FILE*);
void sigTermHandler();
void updateReadPointer();

int main(int argc, char *argv[]){
	signal(SIGTERM, sigTermHandler);
	openFile("frontCamera.data","r", &fileToRead);
	openFile("camera.log", "w", &fileLog);
	updateReadPointer();
	while((socketFd = createConnection("fwcSocket")) < 0)
		usleep(100000);
	startSensor();
}

void updateReadPointer() {
	FILE *fileUtility;
	openFile("utility.data", "r", &fileUtility);
	char *buffer;
	buffer = readLine(fileUtility);
	readPosition = atol(buffer);
	fclose(fileUtility);
	fseek(fileToRead, readPosition, SEEK_SET);
}

void startSensor(){
	char lineaLetta[100];
	while(1){
		char *linea = readLine(fileToRead);
		sendToSocket(socketFd, linea);
		fprintf(fileLog, "%s\n",linea);
		fflush(fileLog);
		sleep(10);
	}
}

char* readLine(FILE *fp){
	int maximumLineLength = 8;
	char *lineBuffer = (char *)malloc(sizeof(char) * maximumLineLength);
	if (lineBuffer == NULL) {
        printf("Errore nell'allocare la memoria del buffer.\n");
        exit(1);
    }
    char ch = getc(fp);
    int count = 0;
    while ((ch != '\n') && (ch != EOF)) {
    	lineBuffer[count] = ch;
        count++;
        ch = getc(fp);
	}
    return lineBuffer;
}

void sigTermHandler() {
	fclose(fileLog);
	FILE *fileUtility;
	openFile("utility.data", "w", &fileUtility);
	readPosition = ftell(fileToRead);
	fprintf(fileUtility, "%ld\n",readPosition);
	fclose(fileUtility);
	fclose(fileToRead);
	exit(0);
}
