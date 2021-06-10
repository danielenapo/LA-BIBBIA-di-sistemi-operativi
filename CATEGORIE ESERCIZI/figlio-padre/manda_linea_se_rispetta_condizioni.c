#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#define PERM 0644

typedef int pipe_t[2];

int main(int argc, char **argv){
	//DEFINIZIONE VARIABILI
	int pid, pidFiglio, status, ritorno;
	int Q, q, j, ret, fd, cfd, numLinee;
	char inpbuf;
	bool finito;
	char linea[250];


	//CONTROLLO PARAMETRI
	if(argc < 3){
		printf("Errore nel numero di parametri\n");
		exit(1);
	}
	Q = argc-1; //numero dei processi figli e relativi file associati

	//CREAZIONE file
	if((cfd=open("Camilla",O_CREAT, PERM))<0){
		puts("Errore creazione file");
		exit(2);
	}


	//ALLOCAZIONE N PIPE DATI
	pipe_t *piped=malloc(sizeof(pipe_t)*(Q));
	if(piped==NULL){
		puts("Errore allocazione array di pipe");
		exit(3);
	}

	//CICLO FIGLI
	for(q=0; q < Q; q++){
		//creazione pipe
		if (pipe (piped[q]) < 0){
			printf("Errore creazione pipe\n");
			exit(4);
		}

		//fork del figlio
		if ((pid = fork()) < 0){
			printf("Errore creazione figlio\n");
			exit(5);
		}


	//CODICE FIGLIO q-ESIMO
	if(pid == 0){
		/*SCHEMA DI COMUNICAZIONE:
		ogni figlio legge una linea alla volta dal file associato e la manda al padre, fino al raggiungimento dell'EOF*/

		//chiudo pipe inutilizzate
		for (int k=0; k<Q; k++){
			close (piped [k][0]);
			if(k!=q){
				close(piped [k][1]);
			}
		}

		//APERTURA FILE ASSOCIATO
		if((fd=open(argv[q+1], O_RDONLY)) < 0){
			printf("Errore in apertura file associato %s\n", argv[q+1]);
							exit(-1);
		}

		//LETTURA DEL FILE
		//leggo un carattere alla volta, salvandolo nel vettore linea fino al raggiungimento del \n. a quel punto mando la linea al padre (se rispetta le condizioni) e leggo una nuova linea
		j=0; //azzero il contatore dei caratteri della linea
		numLinee=0;
		while(read(fd, &inpbuf, 1)){
			linea[j]=inpbuf;
			//quando trovo il terminatore di linea
			if(inpbuf=='\n'){
				//controlli delle condizioni per scrittura al PADRE
				if(j<=10 && (linea[0]>=48 && linea[0]<=57)){
					ret=write(piped[q][1], &j, sizeof(int));
					if(ret!=sizeof(int)){
						perror("Errore scrittura lunghezza linea\n");
						exit(-1);
					}
					ret=write(piped[q][1], linea, j);//scrivo la linea al Padre
				//controllo che la scrittura sia andata bene
				if(ret!=j){
					perror("Errore scrittura in pipe dal figlio\n");
					exit(-1);
				}
				numLinee++;//incremento il contatore di linee
			}
				j=0;//azzero il contatore per la prossima linea
			}
			else
				j++;//altrimenti incremento il contatore
		}

		exit(numLinee);
	}
}



//PADRE
//chiusura pipe inutilizzate
	for(int k=0 ; k<Q; k++){
		close (piped[k][1]);
	}

	printf("Padre %d sta per iniziare a leggere i messaggi della pipe \n", getpid());

	finito=false;
	while(finito==false){
		finito=true; //suppongo che siano finiti tutti
		for(int i=0; i<Q; i++) {
				//legge dalla pipe la lunghezza della linea e poi la relativa linea
				ret=read(piped[i][0],&j, sizeof(int));
				ret=read(piped[i][0],linea, j);

				if(ret==j){
					//se ha letto correttamente la linea del figlio i, significa che almeno un figlio non ha ancora finito di leggere il file, quindi devo continuare col while
					finito=false;
					linea[j]=0; //la trasformo in stringa
					printf("FIGLIO: %d FILE: %s LINEA: %s\n",i, argv[i+1], linea);
				}


			}
		}

//ASPETTA TUTTI I FIGLI
	for(int c=0; c<Q; c++){
        pidFiglio = wait(&status);
        if (pidFiglio < 0){
            printf("Errore wait\n");
            exit(7);
        }

        if ((status & 0xFF) != 0)
            printf("Figlio con pid %d terminato in modo anomalo\n", pidFiglio);
        else{
            ritorno=(int)((status >> 8) & 0xFF);
            printf("Il figlio con pid=%d ha ritornato %d (se 255 problemi!)\n", pidFiglio,ritorno);
        }
    }
		exit (0);
}
