#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <time.h>

#define PERM 0664

int main(int argc, char **argv){
	srand(time(NULL));
	int pid, pidFiglio, status, ritorno, fd;
	int lineeInviate=0, contaCarLinea=0;;//si suppongono minori di 255 e 250
	int Q= argc-1;
	char ch;
	char linea[250]; //si suppone minore di 250
	typedef int pipe_t[2];

	//CONTROLLO MUMERO DI PARAMETRI
	if(argc < 3){
		printf("Errore nel numero di parametri\n");
		exit(1);
	}

	//CREAZIONE FILE
	int fdTemp=open("Camilla", O_WRONLY | O_CREAT | O_TRUNC ,PERM);

	//ALLOCAZIONE Q PIPE
	pipe_t *piped=malloc(sizeof(pipe_t)*(Q));

	//SCORRO TUTTI I FIGLI
	for(int q=0; q < Q; q++){
		//CREAZIONE Q PIPE
		if (pipe (piped[q]) < 0){
			printf("Errore creazione\n");
			exit(3);
		}
		//CREAZIONE Q FIGLI
		if ((pid = fork()) < 0){
			printf("Errore creazione figlio\n");
			exit(4);
		}

	//CODICE FIGLIO q-ESIMO
	if(pid == 0){
		//CHIUSURA PIPE INUTILIZZATE
		for (int k=0; k<Q; k++){
			close (piped [k][0]);
			if(k!=q){
				close(piped [k][1]);
			}
		}
		//APERTURA FILE ASSOCIATO AL FIGLIO
		if((fd=open(argv[q+1], O_RDONLY)) < 0){
			printf("Errore in apertura file %s\n", argv[q+1]);
							exit(-1);
		}

		/* LETTURA FILE */
		while((read(fd, &ch, 1))!=0){
			//assegno il carattere alla stringa e incremento il contatore di caratteri
			linea[contaCarLinea]=ch;
			contaCarLinea++;
			//quando arriviamo alla fine della linea
			if(ch=='\n'){
				if(linea[0]>=48 && linea[0] <=57 && contaCarLinea<10){ //controllo che il primo carattere sia numerico, e che la lunghezza della linea sia <10 (compreso \n)
					//in caso positivo scrivo in pipe la linghezza della linea, la linea e incremento il contatore delle linee
					write(piped[q][1],&contaCarLinea, sizeof(int));
					write(piped[q][1],linea, contaCarLinea*sizeof(char));
					lineeInviate++;
				}
				contaCarLinea=0; //in ogni caso azzero il contatore di caratteri della linea
			}

		}
		exit(lineeInviate); //ritorno il contatore delle linee
	}
}


	//PADRE
	//chiude tutte le pipe inutilizzate
	for(int k=0 ; k<Q; k++){
		close (piped[k][1]);
	}

	printf("Padre %d sta per iniziare a leggere i messaggi della pipe \n", getpid());
	//il for garantisce di rispettare l'ordine dei file. scorriamo tutti i processi figli
	for(int q=0; q<Q; q++) {
			//legge il numero di caratteri della linea e la linea stessa
			//metto un while perchè NON SO QUANTE LINEE MI PASSA IL FIGLIO!!!
			while((read(piped[q][0], &contaCarLinea,sizeof(int)))>0){
				read (piped[q][0], linea, sizeof(char)*contaCarLinea);
				//STAMPO INDICE FIGLIO, NOME FILE E LINEA
				printf("FIGLIO: %d, FILE: %s\n LINEA: %s\n",q, argv[q+1],linea);
			}
	}

	//ASPETTA LA TERMINAZIONE DI TUTTI I FIGLI
	for(int c=0; c<Q; c++){
        pidFiglio = wait(&status);
        if (pidFiglio < 0){
            printf("Errore wait\n");
            exit(5);
        }

        if ((status & 0xFF) != 0)
            printf("Figlio con pid %d terminato in modo anomalo\n", pidFiglio);
        else{
            ritorno=(int)((status >> 8) & 0xFF);
            printf("Il figlio con pid=%d ha ritornato %d (se 255 problemi!)\n", pidFiglio,ritorno); //stamo pid e valore di ritorno del figlio c-esimo
        }
    }
		close(fdTemp);
		exit (0);
}
