#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h> 
#include "socketCommunication.h"
#include "fileManagement.h"

#define MAX_BYTE 16


unsigned char buffer[MAX_BYTE];
FILE *fileRead;
FILE *fileLog;
int socketFd;
int stop = 0;


void init(char **mode);
void readAndLog();
void writeOnLog();
void sigStartHandler();
void sigStopHandler();
void sigTermHandler();
void start();




int main (int argc, char *argv[]){
	signal(SIGUSR1,sigStartHandler);
	signal(SIGTERM,sigTermHandler);
	init(&argv[1]);
	while (1)
		pause();
}

void init(char **mode){

    if (strcmp(*mode, "NORMALE") == 0) {
		openFile("/dev/urandom", "rb", &fileRead);
	} else {
		openFile("urandomARTIFICIALE.binary", "rb", &fileRead);
    }
    openFile("spot.log","w",&fileLog);
	while((socketFd = createConnection("bsSocket")) < 0){
		usleep(100000); // 0.1 sec
	}

}

void writeOnLog() {
	for (int c = 0; c < MAX_BYTE; c++) {
		fprintf(fileLog, "%.2X", buffer[c]);
	}
	fprintf(fileLog, "\n");
}

void readAndLog(){
	int qtaRead = 0;
	while (stop == 0){
		qtaRead = fread(buffer,1,MAX_BYTE,fileRead);

        	if(qtaRead == 0){
				perror("blindspot: errore di lettura\n");
				exit(1);
        	}

       		sendToSocket(socketFd, buffer);
        	writeOnLog();
        	fflush(fileLog);
        	usleep(500000); // usleep prende come argomento microsecondi, 0.5 sec = 500000 microsec
    	}

	fprintf(fileLog, "\n");
	stop = 0;
}

void sigStartHandler() {
	signal(SIGUSR1,sigStopHandler);
	readAndLog();
}

void sigStopHandler(){
	signal(SIGUSR1,sigStartHandler);
	stop = 1;
}

void sigTermHandler(){
	fclose(fileRead);
	fclose(fileLog);
	exit(0);
}
