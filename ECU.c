#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/un.h> /* For AFUNIX sockets */
#include <unistd.h>
#include <fcntl.h>
#include "socketCommunication.h"
#include "fileManagement.h"
#include "byteComparison.h"


#define DEFAULT_PROTOCOL 0
#define MAX_SOCKET_NAME_SIZE 30
#define SERVER_SOCKET_NUMBER 5
#define CLIENT_SOCKET_NUMBER 3
#define MAX_DATA_SIZE 100
#define READ 0
#define WRITE 1
#define ATTUATORI_NUMBER 3 //sbw, tc, bbw
#define SENSORI_NUMBER 3 // fwc, ffr, pa
#define DEBUG 1

char sockets_name[][MAX_SOCKET_NAME_SIZE]={
"ffrSocket",
"paSocket",
"bsSocket",
"fwcSocket",
"svcSocket"
};

int ecuServerSocketArray[SERVER_SOCKET_NUMBER];
int ecuClientSocketArray[CLIENT_SOCKET_NUMBER]; // 0 = tc, 1 = bbw, 2 = sbw

pid_t listenersPid[SERVER_SOCKET_NUMBER];
pid_t attuatoriPid[ATTUATORI_NUMBER]; // pid array of attuatori, 0 = sbw, 1 = tc, 2 = bbw
pid_t sensoriPid[SENSORI_NUMBER];// 0 = fwc, 1 = ffr, 2 = pa
pid_t ffrClientPid;
pid_t fwcClientPid;
pid_t bsClientPid;
pid_t hmiCommunicationPid;
pid_t fatherPid;
pid_t hmiPid;

struct sockaddr_un serverUNIXAddress; /*Server address */
struct sockaddr* serverSockAddrPtr; /*Ptr to server address*/
struct sockaddr_un clientUNIXAddress; /*Client address */
struct sockaddr* clientSockAddrPtr;/*Ptr to client address*/
int serverLen, clientLen;

int PIPE_server_to_fwc_manager[2];
int PIPE_server_to_ffr_manager[2];
int PIPE_server_to_hmi_manager[2];
int PIPE_server_to_bs_manager[2];
int PIPE_server_to_pa_manager[2];

int currentSpeed;

void init();
void start(char**);
void fullServer();

void ecuFwcClient();
void ecuFwcClientCreateSockets();
void elaborateFwcData(char*);
void managePericoloToBbw();

void ecuFfrClient();
void elaborateFfrData(char*);

void ecuBsClient();
void hmiCommunication();

int readFromPipe(int, char*);
void sendDataToSocket(int, char*);
void closeSystem();
int extractString(char*);
void closeAllPipes();

void sigstartHandler();
void sigparkHandler();
void sigParkEndHandler();
void sigParkCallHandler(); // moha

int main(int argc, char *argv[]){
	signal(SIGUSR1, sigstartHandler);
	hmiPid = getppid();
	fatherPid = getpid();
	init(); // inizializzazione
	pause(); //blocca il processo
	start(&argv[1]); //la ecu esegue le sue funzioni normali
}

void init() {
	currentSpeed = 0;

 	serverSockAddrPtr = (struct sockaddr*) &serverUNIXAddress;
 	serverLen = sizeof (serverUNIXAddress);
 	clientSockAddrPtr = (struct sockaddr*) &clientUNIXAddress;
 	clientLen = sizeof (clientUNIXAddress);
 	serverUNIXAddress.sun_family = AF_UNIX;

 	pipe(PIPE_server_to_fwc_manager);
	pipe(PIPE_server_to_ffr_manager);
	pipe(PIPE_server_to_hmi_manager);
	pipe(PIPE_server_to_bs_manager);
	pipe(PIPE_server_to_pa_manager);

	printf("Sistema inizializzato.\n\n");
}

void start(char **mode) {
	int i = 0;
	int j = 0;
	char *argv[6];
	attuatoriPid[i] = fork();
	if(attuatoriPid[i] == 0) { // child STEER BY WIRE
		closeAllPipes();
		argv[0] = "./sbw";
		argv[1] = *mode;
		execv(argv[0],argv);
		return;
	} else { // father
		i++; // 1
		attuatoriPid[i] = fork();
		if (attuatoriPid[i] == 0) { // child THROTTLE CONTROL
			closeAllPipes();
			argv[0] = "./tc";
			argv[1] = *mode;
			execv(argv[0],argv);
			return;
		} else { // father
			i++; // 2
			attuatoriPid[i] = fork();
			if(attuatoriPid[i] == 0) { // child BRAKE BY WIRE
				closeAllPipes();
				argv[0] = "./bbw";
				argv[1] = *mode;
				execv(argv[0],argv);
				return;
			} else { // father
				fwcClientPid = fork();
				if(fwcClientPid == 0) { // child ecuFwcClient
					close(PIPE_server_to_ffr_manager[WRITE]);
					close(PIPE_server_to_ffr_manager[READ]);
					close(PIPE_server_to_pa_manager[WRITE]);
					close(PIPE_server_to_pa_manager[READ]);
					close(PIPE_server_to_bs_manager[WRITE]);
					close(PIPE_server_to_bs_manager[READ]);
					close(PIPE_server_to_fwc_manager[WRITE]);
 					ecuFwcClient();
				} else {
					bsClientPid = fork();// father
					if(bsClientPid == 0){
						close(PIPE_server_to_hmi_manager[WRITE]);
						close(PIPE_server_to_hmi_manager[READ]);
						close(PIPE_server_to_ffr_manager[WRITE]);
						close(PIPE_server_to_ffr_manager[READ]);
						close(PIPE_server_to_pa_manager[WRITE]);
						close(PIPE_server_to_pa_manager[READ]);
						close(PIPE_server_to_fwc_manager[WRITE]);
						close(PIPE_server_to_fwc_manager[READ]);
						close(PIPE_server_to_bs_manager[WRITE]);
						ecuBsClient();
					} else {
						sensoriPid[j] = fork();
						if(sensoriPid[j] == 0) { // child FRONT WINDSHIELD CAMERA
							close(PIPE_server_to_hmi_manager[WRITE]);
							close(PIPE_server_to_hmi_manager[READ]);
							close(PIPE_server_to_ffr_manager[WRITE]);
							close(PIPE_server_to_ffr_manager[READ]);
							close(PIPE_server_to_pa_manager[WRITE]);
							close(PIPE_server_to_pa_manager[READ]);
							close(PIPE_server_to_fwc_manager[WRITE]);
							close(PIPE_server_to_fwc_manager[READ]);
							close(PIPE_server_to_bs_manager[WRITE]);
							close(PIPE_server_to_bs_manager[READ]);
							argv[0] = "./fwc";
							argv[1] = *mode;
							execv(argv[0],argv);
							return;
						} else { // father
							j++; // 1
							sensoriPid[j] = fork();
							if(sensoriPid[j] == 0) { // child FORWARD FACING RADAR
								close(PIPE_server_to_hmi_manager[WRITE]);
								close(PIPE_server_to_hmi_manager[READ]);
								close(PIPE_server_to_ffr_manager[WRITE]);
								close(PIPE_server_to_ffr_manager[READ]);
								close(PIPE_server_to_pa_manager[WRITE]);
								close(PIPE_server_to_pa_manager[READ]);
								close(PIPE_server_to_fwc_manager[WRITE]);
								close(PIPE_server_to_fwc_manager[READ]);
								close(PIPE_server_to_bs_manager[WRITE]);
								close(PIPE_server_to_bs_manager[READ]);
								argv[0] = "./ffr";
								argv[1] = *mode;
								execv(argv[0],argv);
								return;
							} else { // father
								j++; // 2
								sensoriPid[j] = fork();
								if (sensoriPid[j] == 0) { // child PARK ASSIST
									close(PIPE_server_to_hmi_manager[WRITE]);
									close(PIPE_server_to_hmi_manager[READ]);
									close(PIPE_server_to_ffr_manager[WRITE]);
									close(PIPE_server_to_ffr_manager[READ]);
									close(PIPE_server_to_pa_manager[WRITE]);
									close(PIPE_server_to_pa_manager[READ]);
									close(PIPE_server_to_fwc_manager[WRITE]);
									close(PIPE_server_to_fwc_manager[READ]);
									close(PIPE_server_to_bs_manager[WRITE]);
									close(PIPE_server_to_bs_manager[READ]);
									argv[0] = "./pa";
									argv[1] = *mode;
                       				execv(argv[0],argv);
									return;
								} else {
									ffrClientPid = fork();
									if(ffrClientPid == 0) {
										close(PIPE_server_to_hmi_manager[WRITE]);
										close(PIPE_server_to_hmi_manager[READ]);
										close(PIPE_server_to_pa_manager[WRITE]);
										close(PIPE_server_to_pa_manager[READ]);
										close(PIPE_server_to_fwc_manager[WRITE]);
										close(PIPE_server_to_fwc_manager[READ]);
										close(PIPE_server_to_bs_manager[WRITE]);
										close(PIPE_server_to_bs_manager[READ]);
										close(PIPE_server_to_ffr_manager[WRITE]);
										ecuFfrClient();
									} else {
										fullServer();
									}
								}
							}
						}
					}
				}
			}
		}
	}
}

void fullServer(){

	int socketFd, clientFd;

	for(int i = 0; i < SERVER_SOCKET_NUMBER; i++){
		listenersPid[i] = fork();
			if(listenersPid[i] == 0){
				ecuServerSocketArray[i] = socket (AF_UNIX, SOCK_STREAM, DEFAULT_PROTOCOL);
				strcpy (serverUNIXAddress.sun_path, sockets_name[i]);
				unlink (sockets_name[i]);
				bind (ecuServerSocketArray[i], serverSockAddrPtr, serverLen);
				listen (ecuServerSocketArray[i], 1);
				while (1) {/* Loop forever */ /* Accept a client connection */
		   			socketFd = accept (ecuServerSocketArray[i], clientSockAddrPtr, &clientLen);
		   			char str[200];
		   			while(readFromSocket(socketFd, str)){
		   				if(strcmp(sockets_name[i], "fwcSocket")== 0){//controlla che figlio è
		   					write(PIPE_server_to_fwc_manager[WRITE], str, strlen(str)+1);
							} //Invia alla pipe che gestisce gli fwc.
							else if(strcmp(sockets_name[i], "ffrSocket") == 0){
							 		write(PIPE_server_to_ffr_manager[WRITE], str, strlen(str)+1);
							}
							else if(strcmp(sockets_name[i], "bsSocket") == 0){
									write(PIPE_server_to_bs_manager[WRITE], str, strlen(str)+1);
							}else if(strcmp(sockets_name[i], "paSocket") == 0){
									write(PIPE_server_to_pa_manager[WRITE], str, strlen(str)+1);
							}
			 		}
			 		close (socketFd); /* Close the socket */
			 		exit (0); /* Terminate */
		   		}
			}
		}
		close(PIPE_server_to_ffr_manager[WRITE]);
		close(PIPE_server_to_ffr_manager[READ]);
		close(PIPE_server_to_fwc_manager[WRITE]);
		close(PIPE_server_to_fwc_manager[READ]);
		close(PIPE_server_to_bs_manager[WRITE]);
		close(PIPE_server_to_bs_manager[READ]);
		close(PIPE_server_to_pa_manager[WRITE]);
		close(PIPE_server_to_pa_manager[READ]);
		hmiCommunication();
}

// ######################### ECU FWC CLIENT ####################################

void ecuFwcClient(){

	while((ecuClientSocketArray[0] = createConnection("tcSocket")) < 0) //attende di connettersi alle socket degli attuatori
	 	usleep(100000);

	while ((ecuClientSocketArray[1] = createConnection("bbwSocket")) < 0)
		usleep(100000);

	while ((ecuClientSocketArray[2] = createConnection("sbwSocket")) < 0)
		usleep(100000);

	char dataReceived[MAX_DATA_SIZE];
	while(1){
		read(PIPE_server_to_fwc_manager[READ], dataReceived,  MAX_DATA_SIZE);
		elaborateFwcData(dataReceived);
	}
}

void elaborateFwcData(char *data) {
	char command[30];
	sprintf(command, "%s", "NO COMMAND");
	if(strcmp(data, "SINISTRA")==0 || strcmp(data, "DESTRA") ==0) {
		sprintf(command, "%s", data);
		sendDataToSocket(ecuClientSocketArray[2], data); //invia a steer-by-wire
	} else if(strcmp(data, "PERICOLO")==0) {
		managePericoloToBbw();
	} else { //ecuServer has received speed value
		int receivedSpeed = atoi(data);
		if(receivedSpeed < currentSpeed){
			sprintf(command, "%s%d", "FRENA ", (currentSpeed-receivedSpeed));
			currentSpeed = receivedSpeed;
			sendDataToSocket(ecuClientSocketArray[1], command);//invia a throttle-control e break-by-wire	la velocita
		}else if(receivedSpeed > currentSpeed){
			sprintf(command, "%s%d", "INCREMENTO ", (receivedSpeed-currentSpeed));
			currentSpeed = receivedSpeed;
			sendDataToSocket(ecuClientSocketArray[0], command);
		}
		char updatedSpeed[10];
		sprintf(updatedSpeed, "%s %d", "#", receivedSpeed);
		write(PIPE_server_to_hmi_manager[WRITE], updatedSpeed, strlen(updatedSpeed)+1);
	}
		//controlla se il comando è vuoto, se non lo è lo invia a HmiCommunicator
		if(strcmp(command, "NO COMMAND") != 0){
			write(PIPE_server_to_hmi_manager[WRITE], command, strlen(command)+1);
		}
}


// ################################### [END] ECU FWC CLIENT ##################################

// ################################### ECU FFR CLIENT #####################################

void ecuFfrClient(){
	char dataReceived[MAX_DATA_SIZE];

	while(1){
		readFromPipe(PIPE_server_to_ffr_manager[READ], dataReceived);
		elaborateFfrData(dataReceived);
	}
}

void elaborateFfrData(char *data){
	if(checkFfrWarning(data) == 1)
		managePericoloToBbw();
}

// ############################## [END] ECU FFR CLIENT #########################

// ############################## ECU BS CLIENT ##################################

void ecuBsClient(){
	char dataReceived[MAX_DATA_SIZE];

	while(1){
		readFromPipe(PIPE_server_to_bs_manager[READ], dataReceived);
		if(checkBsWarning(dataReceived) == 1)
			managePericoloToBbw();
	}

}

// ############################# [END] ECU BS CLIENT ################################

// ############################# HMI COMMUNICATION ##########################

void hmiCommunication(){
	FILE *ecuLog;
	char dataReceived[200];
	int updatedSpeed;
	openFile("ECU.log", "a", &ecuLog);
	while(readFromPipe(PIPE_server_to_hmi_manager[READ], dataReceived)){
		char receivedSpeed[20];
		sprintf(receivedSpeed, "%s", dataReceived);
		if((updatedSpeed = extractString(receivedSpeed)) > -1){
			currentSpeed = updatedSpeed;
		}else{
			fprintf(ecuLog, "%s\n", dataReceived);
			fflush(ecuLog);
		}
	}
}

// ############################ [END] HMI COMMUNICATION ############################

// ############################# UTILITY #####################################

void sendDataToSocket(int socketFd, char *data){
	sendToSocket(socketFd, data);
}

int readFromPipe (int pipeFd, char *data) {
	int n;
	do {
		n = read (pipeFd, data, 1);
	} while (n > 0 && *data++ != '\0');
return (n > 0);
}

void managePericoloToBbw(){
	kill(attuatoriPid[2], SIGUSR1);
	waitpid(attuatoriPid[2], NULL, 0);
	kill(hmiPid, SIGUSR2); //comunico alla hmi di terminare l'intero albero di processi, lei esclusa, e di riavviare il sistema
}

void closeSystem(){
		for(int i = 0; i < ATTUATORI_NUMBER; i++){
				kill(attuatoriPid[i], SIGTERM);
		}
		for(int i = 0; i < SENSORI_NUMBER; i++){
				kill(sensoriPid[i], SIGTERM);
		}
		exit(0); // termino il padre
}

int extractString(char* data) {//estrae l'incremento di velocita' dall'input inviato dall'ECU
	char* updatedSpeed;
	updatedSpeed = strtok (data," ");
	if(strcmp(updatedSpeed, "#") == 0){
		updatedSpeed = strtok (NULL, " ");
		return atoi(updatedSpeed);
	}else
		return -1;
}

void closeAllPipes(){
	close(PIPE_server_to_bs_manager[READ]);
	close(PIPE_server_to_bs_manager[WRITE]);
	close(PIPE_server_to_ffr_manager[READ]);
	close(PIPE_server_to_ffr_manager[WRITE]);
	close(PIPE_server_to_fwc_manager[READ]);
	close(PIPE_server_to_fwc_manager[WRITE]);
	close(PIPE_server_to_pa_manager[READ]);
	close(PIPE_server_to_pa_manager[WRITE]);
	close(PIPE_server_to_hmi_manager[READ]);
	close(PIPE_server_to_hmi_manager[WRITE]);
}
void sigstartHandler() {
	signal(SIGUSR1, sigparkHandler); //reset signal
	return;
}

void sigparkHandler() {
	signal(SIGUSR1, sigParkCallHandler);
	char parkCommand[20];
	sprintf(parkCommand,"%s %d", "PARCHEGGIO", currentSpeed);
	int bbwConne = createConnection("bbwSocket");
	sendDataToSocket(bbwConne, parkCommand);
	write(PIPE_server_to_hmi_manager[WRITE], parkCommand, strlen(parkCommand)+1);
	kill(attuatoriPid[0], SIGTERM);
	kill(attuatoriPid[1], SIGTERM);
	kill(sensoriPid[0], SIGTERM);
	kill(sensoriPid[1], SIGTERM);
}

void sigParkCallHandler(){
	signal(SIGUSR1,sigParkEndHandler);
	kill(sensoriPid[2], SIGUSR1); // avvia procedura di PARCHEGGIO
}

void sigParkEndHandler(){
	kill(hmiPid, SIGUSR1); //comunico alla hmi di terminare l'intero albero di processi, lei inclusa
}
