#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "fileManagement.h"
#include "socketCommunication.h"

FILE *fp;
FILE *logfp;
unsigned char buffer[24];
int socketFd;

void writeOnLog();
void sigTermHandler();

int main(int argc, char **argv) {
		signal(SIGTERM, sigTermHandler);
		if (strcmp(argv[1], "NORMALE") == 0) {
				//avvia lettura da random
				openFile("/dev/random", "rb", &fp);
		} else {
				//avvia lettura da randomARTIFICIALE
				openFile("randomARTIFICIALE.binary", "rb", &fp);
		}
		openFile("radar.log", "w", &logfp);

		while((socketFd = createConnection("ffrSocket")) < 0)
			usleep(100000); //0.1 sec
		int elementsRead;
		while(1) {
				elementsRead = fread(buffer, 1, 24, fp);
				if (elementsRead == 24) {
						sendToSocket(socketFd, buffer);
						writeOnLog();
						fflush(logfp);
				}
				sleep(2);
		}
}

void writeOnLog() {
		for (int c = 0; c < 24; c++)
				fprintf(logfp, "%.2X", buffer[c]);
		fprintf(logfp, "\n");
}

void sigTermHandler() {
	fclose(fp);
	fclose(logfp);
	exit(0);
}
