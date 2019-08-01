#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#include "fileManagement.h"
#include "socketCommunication.h"

#define MAX_BYTE 4
#define DURATION 30

// Variables
FILE *fileRead;
FILE *fileLog;
int socketFd;
unsigned char buffer[MAX_BYTE];
static char *sub_process_name = "./svc";
pid_t pidSvc;
pid_t ecuPid;
int restart = 0;


// Functions
void init(char **mode);
void readAndLog();
void writeOnLog();
void sigStartHandler();
void sigStopHandler();
void sigTermHandler();

int main (int argc, char *argv[]){
	ecuPid = getppid();
	pidSvc = fork();
    if(pidSvc<0) {
        perror("fork");
        return 1;
    }

    if(pidSvc == 0) {
        argv[0] = sub_process_name;
        execv(argv[0],argv);
        return 1;
    } else {
    	signal(SIGUSR1, sigStartHandler);
		signal(SIGTERM,sigTermHandler);
        init(&argv[1]);
        pause();
    }
}

void init(char **mode){

    if (strcmp(*mode, "NORMALE") == 0) {
		openFile("/dev/urandom", "rb", &fileRead);
	} else {
		openFile("urandomARTIFICIALE.binary", "rb", &fileRead);
    }
    openFile("assist.log","w",&fileLog);
    while((socketFd = createConnection("paSocket")) < 0)
        sleep(1);
}

void writeOnLog() {
	for (int c = 0; c < MAX_BYTE; c++) {
		fprintf(fileLog, "%.2X", buffer[c]);
	}
	fprintf(fileLog, "\n");
}

void readAndLog(){
	printf("Inizio procedura di parcheggio\n");

	for (int i = 0; i<DURATION; i++){
		printf("\rParcheggio in corso: %d%%", (i + 1) * 100 / 30);
		fflush(stdout);
        if (fread(buffer,1,MAX_BYTE,fileRead) == 0) {
        	perror("parkAssist: errore di lettura");
        	exit(1);
        }
        if(restart == 1) {
            i = 0;		// se riceve un segnale di restart riparte per altri 30 secondi
            restart = 0;	// resetta restart
        }
        sendToSocket(socketFd, buffer);
        writeOnLog();
        fflush(fileLog);
        sleep(1);
    }
    printf("\n");

}

void sigStartHandler(){
    signal(SIGUSR1,sigStopHandler);
    kill(pidSvc,SIGUSR1);	// AVVIA SorroundViewWithCameras
    readAndLog();
    kill(pidSvc,SIGUSR1); 	// DISATTIVA SorroundViewWithCameras
    kill(ecuPid, SIGUSR1); 	// COMUNICA alla ECU che ha finito
	sigTermHandler();		// MUORE
}

void sigStopHandler() {
    signal(SIGUSR1,sigStopHandler);
    int restart = 1;
}

void sigTermHandler() {
	signal(SIGTERM, SIG_DFL);
    fclose(fileRead);
    fclose(fileLog);
    kill(pidSvc,SIGTERM); 	// kill SVC process
	kill(getpid(), SIGTERM);
}
