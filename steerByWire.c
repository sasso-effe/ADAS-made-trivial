#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/un.h> /* For AFUNIX sockets */
#include <fcntl.h>
#include "socketCommunication.h"
#include "fileManagement.h"
#define DEFAULT_PROTOCOL 0
#define READ 0
#define WRITE 1

// Functions 
int startElaboration();
int extractString();
void startLoop();
void initSocket();
void stoGirando();
void steeringLog();
void sigTermHandler();

// Variables
char *direction = "no direction";
FILE* fileLog;	
int status;	//pipe status
int pfd[2]; //pipe array
pid_t blindSpotPid;
pid_t pid;
int called = 0;

int main(int argc, char* argv[]){
	status = pipe(pfd);
	if(status != 0) {
		printf("Error with pipe\n");
		exit(-1);
	}
	fcntl(pfd[READ], F_SETFL, O_NONBLOCK);	//rende la read non bloccante
	pid = fork();	//crea un processo figlio di scrittura su file di log
	if(pid == 0){ // child PROCESSO CHE LOGGA
		close(pfd[READ]); 
		initSocket();
		

		return 1;
	} else { // father
		blindSpotPid = fork();
		if (blindSpotPid == 0){ // child BLINDSPOT PROCESS
			argv[0] = "./bs"; 
			execv(argv[0],argv); // mette in esecuzione blindspot
			return 1;
		} else { // father
			signal(SIGTERM,sigTermHandler);
			close(pfd[WRITE]);
			startLoop();			
		}
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
 	
 	char* socketName = "sbwSocket";
 	serverFd = socket (AF_UNIX, SOCK_STREAM, DEFAULT_PROTOCOL);
	serverUNIXAddress.sun_family = AF_UNIX;
	strcpy (serverUNIXAddress.sun_path, socketName); 
	unlink (socketName);
	bind (serverFd, serverSockAddrPtr, serverLen);
	listen (serverFd, 1);	 	
	
	while (1) {/* Loop forever */ /* Accept a client connection */
   		clientFd = accept (serverFd, clientSockAddrPtr, &clientLen);
   		if (fork () == 0) { /* Create child */
     		char data[100]; //char *data;
     		while (readFromSocket(clientFd, data)){
     			write(pfd[WRITE], data, 30);
            }
     		close (clientFd); /* Close the socket */
     		exit (/* EXIT_SUCCESS */ 0); /* Terminate */
   		} else
     		close (clientFd); /* Close the client descriptor */
  	}
}

void startLoop() {
	openFile("steer.log", "w", &fileLog);
	while(1) {
		char sentData[30];								
		if (read(pfd[0], sentData, 30) > 0) {
			direction = sentData;
		}

		if (strcmp(direction,"DESTRA") == 0 ||
			strcmp(direction, "SINISTRA") == 0) {
				steeringLog();
		} else {
			fprintf(fileLog, "NO ACTION\n");
			fflush(fileLog);
		}
		sleep(1);
	}
}

void steeringLog(){
	kill(blindSpotPid,SIGUSR1);
	for (int i = 0; i<4; i++) {
		fprintf(fileLog,"sto girando a %s\n", direction);
		fflush(fileLog);
		sleep(1);
	}
	direction = "no direction";
	kill(blindSpotPid, SIGUSR1);

}

void sigTermHandler() {
	signal(SIGTERM,SIG_DFL);
	kill(pid,SIGTERM);
	kill(blindSpotPid,SIGTERM);
	exit(0);
}


