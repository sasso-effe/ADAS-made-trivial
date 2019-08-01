#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h> /* For AFUNIX sockets */
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include "socketCommunication.h"
#include "fileManagement.h"
#define SIGWARNING SIGUSR2
#define SIGPARK SIGUSR1
#define SIGSTART SIGUSR1
#define DEFAULT_PROTOCOL 0
#define MAX_SOCKET_NAME_SIZE 30
#define SOCKET_NUMBER 5
#define FWC_CLIENT_SOCKET_NUMBER 3
#define MAX_DATA_SIZE 100
#define READ 0
#define WRITE 1

static char *sub_process_name = "./ecu";
pid_t ecuPid;
pid_t tailPid;
short int started;
char **g_argv;

void sayHello();
void start();
void startServer();
void readAndLog();
void writeOnLog();
void recreateEcu();
void sigWarningHandler();
void sigParkHandler();

int main (int argc, char *argv[]){
	g_argv = argv;
	if (argc < 2 || (
	strcmp(argv[1], "NORMALE") != 0
	&& strcmp(argv[1], "ARTIFICIALE") != 0)) {
		printf("Specifica se vuoi un avvio NORMALE o ARTIFICIALE\n");
		exit(1);
	}
	ecuPid = fork();
    if(ecuPid<0) {
        perror("fork");
        return 1;
    }
    if(ecuPid == 0) {
    	setpgid(0, 0);
        argv[0] = sub_process_name;
        execv(argv[0], argv);
    } else {
    	tailPid = fork();
    	if(tailPid < 0){
        	perror("fork");
        	return 1;
    	}
    	if(tailPid == 0) {
        	system("rm -f ECU.log; touch ECU.log; gnome-terminal -- sh -c \"echo OUTPUT:; tail -f ECU.log; bash\"");
    	} else {
    		signal(SIGPARK, sigParkHandler);
			signal(SIGWARNING, sigWarningHandler);
			signal(SIGINT, sigParkHandler);
			FILE *fp;			
			openFile("utility.data", "w", &fp);
			fprintf(fp, "%d\n", 0);
			fclose(fp);
						
			openFile("ECU.log", "w", &fp);
			//fprintf(fp, "%d\n", 0);
			fclose(fp);
			
			sayHello();
        	start();
        }
    }

}


void sayHello(){
	char *lgn;
	if((lgn = getlogin()) == NULL) {
		fprintf(stderr, "Get of user name failed.\n");
		exit(1);
	}	
	printf("Ciao %s! Benvenuto nel simulatore di sistemi di guida autonoma. \nDigita INIZIO per avviare il veicolo,\no digita PARCHEGGIO per avviare la procedura di parcheggio e concludere il percorso.\n\n", lgn);
};

void start(){
	char input[30];
	started = 0;	
	while(1) {
		if(fgets(input, 30, stdin) != NULL){
			if((started) == 0) {
				if(strcmp(input, "INIZIO\n") == 0) {
					printf("Veicolo avviato\n");
					kill(ecuPid, SIGSTART);
					started = 1;
				} else if (strcmp(input, "PARCHEGGIO\n") == 0) {
					printf("Prima di poter parcheggiare devi avviare il veicolo.\nDigita INIZIO per avviare il veicolo.\n\n");
				} else {
					printf("Comando non ammesso.\n\n");
				}
			} else {
				if(strcmp(input, "PARCHEGGIO\n") == 0) {
					printf("Sto fermando il veicolo...\n");
					kill(ecuPid, SIGPARK);
					started = 0;
				} else {
					printf("Comando non ammesso. \nDigita PARCHEGGIO per parcheggiare il veicolo\n\n");
				}
			}
		}
	}
	return;
}

void sigWarningHandler() {
	signal(SIGWARNING, sigWarningHandler);
	kill(-ecuPid, SIGTERM);
	recreateEcu();
	printf("La macchina è stata arrestata per evitare un pericolo. \nPremi INIZIO per ripartire\n\n");
	started = 0;
}

void sigParkHandler() {
	kill(-ecuPid, SIGTERM);
	kill(tailPid, SIGTERM);
	kill(0, SIGTERM);
	
}

void sigerrorHandler() {
	exit(0);
}

void recreateEcu() {
	ecuPid = fork();
    if(ecuPid<0) {
        perror("fork");
        exit(1);
    }
    if(ecuPid == 0) {
    	setpgid(0, 0);
        execv(sub_process_name, g_argv);
    }
}
