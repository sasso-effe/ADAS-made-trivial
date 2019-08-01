#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h> /* For AFUNIX sockets */
#include <fcntl.h>
#include "socketCommunication.h"
#include "fileManagement.h"
#define DEFAULT_PROTOCOL 0
#define READ 0
#define WRITE 1
#define SIGPARK SIGUSR1
#define SIGWARNING SIGUSR1

int startElaboration(const char*);
int extractString(char*);
void startLoop();
void initSocket();
void sigTermHandler();
void sigWarningHandler();

int deltaSpeed;
FILE* fileLog;
int status;	//pipe status
int pfd[2]; //pipe array
pid_t pid;
pid_t ecuPid;
char* command;

int main(void){
	ecuPid = getppid();
	status = pipe(pfd);
	if(status != 0) {
		printf("Error with pipe\n");
		exit(-1);
	}
	openFile("brake.log", "w", &fileLog);
	fcntl(pfd[0], F_SETFL, O_NONBLOCK);	//rende la read non bloccante
	pid = fork();					//crea un processo figlio di scrittura
	if(pid == 0) {
		close(pfd[WRITE]);
		startLoop();
		close(pfd[READ]);
		return 1;
	} else {
		signal(SIGWARNING, sigWarningHandler);
		signal(SIGTERM, sigTermHandler);
		close(pfd[READ]);
		initSocket();					//il padre rimane in ascolto sulla socket
	}
}

void initSocket() {
	int serverFd, clientFd, serverLen, clientLen;
 	struct sockaddr_un serverUNIXAddress; /*Server address */
  	struct sockaddr* serverSockAddrPtr; /*Ptr to server address*/
  	struct sockaddr_un clientUNIXAddress; /*Client address */
 	struct sockaddr* clientSockAddrPtr;/*Ptr to client address*/

 	serverSockAddrPtr = (struct sockaddr*) &serverUNIXAddress;
 	serverLen = sizeof (serverUNIXAddress);
 	clientSockAddrPtr = (struct sockaddr*) &clientUNIXAddress;
 	clientLen = sizeof (clientUNIXAddress);
	serverUNIXAddress.sun_family = AF_UNIX;
 	char* socketName = "bbwSocket";

 	serverFd = socket (AF_UNIX, SOCK_STREAM, DEFAULT_PROTOCOL);
	strcpy (serverUNIXAddress.sun_path, socketName);
	unlink (socketName);
	bind (serverFd, serverSockAddrPtr, serverLen);
	listen (serverFd, 2);

	while (1) {/* Loop forever */ /* Accept a client connection */
   		clientFd = accept (serverFd, clientSockAddrPtr, &clientLen);
   		if (fork () == 0) { /* Create child */
     		char data[100];
     		while (readFromSocket(clientFd, data)){
					startElaboration(data);
				}
     		close (clientFd); /* Close the socket */
     		exit (/* EXIT_SUCCESS */ 0); /* Terminate */
   		}else
			 close (clientFd); /* Close the client descriptor */
  	}
}

int startElaboration(const char* data) {
	deltaSpeed = extractString(strdup(data));
	int waitingTime = deltaSpeed/5;
	write(pfd[1], &deltaSpeed, sizeof(deltaSpeed));
	if (strcmp(command, "PARCHEGGIO") == 0) {
		sleep(waitingTime);
		kill(ecuPid, SIGPARK); // SEGNALA CHE HA PARCHEGGIATO
		sigTermHandler(); // MUORE

	}
}

int extractString(char* data) {							//estrae il decremento di velocita' dall'input inviato dall'ECU
	char* acceleration;
	command = strtok (data," ");
	acceleration = strtok (NULL, " ");					//split tramite " "
	return (int) strtol(acceleration, NULL, 10);		//converte la stringa in intero
}

void startLoop() {
	while(1) {
		int sentData;									//variabile d'appoggio per la lettura da pipe
		if (read(pfd[0], &sentData, sizeof(sentData)) > 0) {
			deltaSpeed = sentData;
		}
		if (deltaSpeed > 0) {
			fprintf(fileLog, "DECREMENTO 5\n");
			fflush(fileLog);
			deltaSpeed = deltaSpeed - 5;
		} else {
			fprintf(fileLog, "NO ACTION\n");
			fflush(fileLog);
		}
		sleep(1);
	}
}

void sigWarningHandler() {
	fprintf(fileLog, "ARRESTO AUTO\n");
	kill(getpid(), SIGTERM);
}

void sigTermHandler() {
	kill(pid, SIGTERM);
	exit(0);
}
