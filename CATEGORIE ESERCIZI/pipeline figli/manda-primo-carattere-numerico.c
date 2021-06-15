#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>

typedef struct{
	int c1; //pid
	long int c2; //numero corrispondente al carattere trovato
}strut;

typedef int pipe_t[2];

int main(int argc, char **argv){
	int  pidFiglio, status,ret,fd; //gestione comunicazione pipe e valore di ritorno
	char inpbuf; //lettura caratteri dal file
  int n; //num di figli
	long somma;//somma di tutti i numeri trovati


	//CONTROLLO PARAMETRI
	if(argc < 2){
		printf("Errore nel numero di parametri\n");
		exit(1);
	}
	n = argc-1; //numero dei processi figli

	//array pidFiglio
	int *pid=malloc(sizeof(int)*n);
	if(pid==NULL){
		puts("Errore allocazione");
		exit(2);
	}
	//CREAZIONE N PIPE
	pipe_t *piped=malloc(sizeof(pipe_t)*(n));
	if(piped==NULL){
		puts("Errore allocazione");
		exit(3);
	}

	//CICLO FIGLI
	for(int c=0; c < n; c++){
		if (pipe (piped[c]) < 0){
			printf("Errore creazione pipe\n");
			exit(4);
		}


		if ((pid[c] = fork()) < 0){
			printf("Errore creazione figlio\n");
			exit(5);
		}


	//FIGLIO C-ESIMO
	if(pid[c] == 0){
		//COMMENTO CON SPIEGAZIONE UTILIZZO PIPE
		//SCHEMA DI COMUNICAZIONE: ogni figlio manda al successivo il suo array di strut

		//chiudo pipe inutilizzate
		for (int k=0; k<n; k++){
			if(k!=(c-1))
				close (piped [k][0]);
			if(k!=c)
				close(piped [k][1]);
		}

		//array di struttura, con dimensione del contatore (+1 perchè parte da 0)
		strut *S=malloc(sizeof(strut)*(c+1));
		if(S==NULL){
			puts("Errore allocazione");
			exit(-1);
		}
		//inizializzazione struct
		S[c].c1=getpid();
		S[c].c2=0;

		//SE NON E' IL PRIMO FIGLIO, LEGGO IN PIPE DA QUELLO PRECEDENTE
		if(c!=0){
			ret=read(piped[c-1][0],S,sizeof(strut)*c);
			if(ret!=sizeof(strut)*c){
				puts("Errore lettura pipe figlio");
				exit(-1);
			}
		}

		//CONTA OCCORRENZE DAL FILE
		//apertura file associato
		if((fd=open(argv[c+1],O_RDONLY))<0){
			puts("Errore apertura file");
			exit(-1);
		}
		//conta occorrenze
		while(read(fd,&inpbuf,1)!=0){
			if(inpbuf>=48 && inpbuf <= 57){
				S[c].c2=inpbuf-48;
        break;
      }
		}

		//TUTTI I FIGLI SCRIVONO IN PIPE AL SUCCESSIVO IL PROPRIO ARRAY DI STRUCT (l'ultimo manderà al padre)
		ret=write(piped[c][1],S,sizeof(strut)*(c+1));
    if(ret!=sizeof(strut)*(c+1)){
      perror("Errore scrittura pipe");
      exit(-1);
    }

		exit(inpbuf); //torno il numero
	}
}


//PADRE
	printf("Padre %d sta per iniziare a leggere i messaggi della pipe \n", getpid());

	//chiusura pipe inutilizzate
	for(int k=0 ; k<n; k++){
		close (piped[k][1]);
		if(k!=(n-1))
			close (piped[k][0]);
	}

	//array di struttura, con dimensione n
	strut *S=malloc(sizeof(strut)*n);
	if(S==NULL){
		puts("Errore allocazione");
		exit(6);
	}

	//legge l'array di strutture dell'ultimo FIGLIO
	ret=read(piped[n-1][0],S,sizeof(strut)*n);
	if(ret!=sizeof(strut)*n){
		puts("Errore lettura pipe padre");
		exit(7);
	}
	//stampa contenuto struct per ogni figlio, e fa somma di tutti i numeri
  somma=0;
	for(int i=0; i<n; i++){
		printf("INDICE: %d, PID: %d, NUMERO: %ld\n",i, S[i].c1, S[i].c2);
    somma+=S[i].c2;
  }
  printf("SOMMA TOTALE: %ld\n", somma);


//ASPETTA TUTTI I FIGLI
	for(int c=0; c<n; c++){
        pidFiglio = wait(&status);
        if (pidFiglio < 0){
            printf("Errore wait\n");
            exit(8);
        }

        if ((status & 0xFF) != 0)
            printf("Figlio con pid %d terminato in modo anomalo\n", pidFiglio);
        else{
            ret=(int)((status >> 8) & 0xFF);
            printf("Il figlio con pid=%d ha ritornato %c (se 255 problemi!)\n", pidFiglio,ret);
        }
    }
		exit (0);
}
