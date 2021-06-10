#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>

void passavalore(int signo){
	//printf("PORCO DIOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO\n");
	signal(SIGUSR1,passavalore);
}

int main(int argc, char **argv){
	int pid;
	signal(SIGUSR1, passavalore);


		if ((pid = fork()) < 0){
			printf("Errore creazione figlio\n");
			exit(4);
		}

		//pipe(piped);


	//FIGLIO
	if(pid == 0){
		//CODICE
		for(;;){
			printf("FIGLIO\n");
			pause();
			sleep(1);
			kill(getppid(),SIGUSR1);

		}
	}
	else{
	//PADRE
		for(;;){
			printf("PADRE\n");
				//manda segnale al figlio i-esimo
				sleep(1);
				kill(pid,SIGUSR1);
				pause();
				//pause();
				//read(piped[0],&chLetto,1);

				//legge dalla pipe il CARATTERE
		}
	}
}
