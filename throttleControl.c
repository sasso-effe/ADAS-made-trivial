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

int startElaboration();
int extractString();
void startLoop();
void initSocket();
void sigTermHandler();

int deltaSpeed;
FILE* fileLog;	
int status;	//pipe status
int pfd[2]; //pipe array
pid_t pid;

int main(void){
	status = pipe(pfd);
	if(status != 0) {
		printf("Error with pipe\n");
		exit(1);
	}
	fcntl(pfd[READ], F_SETFL, O_NONBLOCK);	//rende la read non bloccante
	pid = fork();							//crea un processo figlio di scrittura su file di log
	if(pid == 0) {
		close(pfd[WRITE]);
		startLoop();
		close(pfd[READ]);
		return 0;
	} else {
		signal(SIGTERM, sigTermHandler);
		close(pfd[READ]); 
		initSocket();						//il padre rimane in ascolto sulla socket
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
 	
 	char* socketName = "tcSocket";
 	serverFd = socket (AF_UNIX, SOCK_STREAM, DEFAULT_PROTOCOL);
	serverUNIXAddress.sun_family = AF_UNIX;
	strcpy (serverUNIXAddress.sun_path, socketName); 
	unlink (socketName);
	bind (serverFd, serverSockAddrPtr, serverLen);
	listen (serverFd, 1);	 	
	
	while (1) {/* Loop forever */ /* Accept a client connection */
   		clientFd = accept (serverFd, clientSockAddrPtr, &clientLen);
   		if (fork() == 0) { /* Create child */
     		char data[30];
     		while (readFromSocket(clientFd, data))
     			startElaboration(data);
     		close (clientFd); /* Close the socket */
     		exit (/* EXIT_SUCCESS */ 0); /* Terminate */
   		} else
     		close (clientFd); /* Close the client descriptor */
  	}
}

int startElaboration(const char* data) {
	deltaSpeed = extractString(strdup(data));
	write(pfd[WRITE], &deltaSpeed, 30);					//scrive l'incremento di velocita' sulla pipe
}

int extractString(char* data) {							//estrae l'incremento di velocita' dall'input inviato dall'ECU
	char* acceleration;
	acceleration = strtok (data," ");
	acceleration = strtok (NULL, " ");					//split tramite " "
	return (int) strtol(acceleration, NULL, 10);		//converte la stringa in intero
}

void startLoop() {
	openFile("throttle.log", "w", &fileLog);
	while(1) {
		int sentData;									//variabile d'appoggio per la lettura da pipe
		if (read(pfd[0], &sentData, 30) > 0) {
			deltaSpeed = sentData;
		}
		if (deltaSpeed > 0) {
			fprintf(fileLog, "AUMENTO 5\n");
			fflush(fileLog);
			deltaSpeed = deltaSpeed - 5;
		} else {
			fprintf(fileLog, "NO ACTION\n");
			fflush(fileLog);
		}
		sleep(1);
	}
}

void sigTermHandler() {
	kill(pid, SIGTERM);
	exit(0);
	
}
