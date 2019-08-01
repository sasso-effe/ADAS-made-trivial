#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "fileManagement.h"
#include "socketCommunication.h"

#define MAX_BYTE 8

// Variables
FILE *fileRead;
FILE *fileLog;
int socketFd;
int stop = 0;
unsigned char buffer[MAX_BYTE];

// Functions 
void start(char **mode);
void readAndLog();
void writeOnLog();
void sigStartHandler(); 
void sigStopHandler(); 
void sigTermHandler(); 

int main (int argc, char *argv[]) {
    signal(SIGUSR1,sigStartHandler); 
    signal(SIGTERM,sigTermHandler); 
	start(&argv[1]);
}

void start(char **mode) {
    if (strcmp(*mode, "NORMALE") == 0) {
		openFile("/dev/urandom", "rb", &fileRead);
	} else {
		openFile("urandomARTIFICIALE.binary", "rb", &fileRead);
    }
    openFile("cameras.log","w",&fileLog);
		while((socketFd = createConnection("svcSocket")) < 0)
			usleep(100000);
		pause();
}

void writeOnLog() {
	for (int c = 0; c < MAX_BYTE; c++) {
		fprintf(fileLog, "%.2X", buffer[c]);
	}
	fprintf(fileLog, "\n");
}

void readAndLog() {
    int qtaRead = 0;
    while (stop == 0){
        qtaRead = fread(buffer,1,MAX_BYTE,fileRead);
        if(qtaRead == 0)
            break;
        sendToSocket(socketFd, buffer);
        writeOnLog();
        fflush(fileLog);
        sleep(1);
    }
    fprintf(fileLog, "\n");
    stop = 0;
}

void sigStartHandler() {
    signal(SIGUSR1,sigStopHandler); // inizializza gestione di stop
    readAndLog(); // avvia il log
}

void sigStopHandler() {
    signal(SIGUSR1,sigStartHandler); // inizializza la gestione di start
    stop = 1;
}

void sigTermHandler() {
	signal(SIGTERM,SIG_DFL);
    fclose(fileRead);
    fclose(fileLog);
	kill(getpid(),SIGTERM);
}
