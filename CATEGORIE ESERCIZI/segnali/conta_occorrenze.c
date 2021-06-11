#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>

void handler(int signo){	//funzione handler di SIGUSR1, nnon fa niente ma evita di terminare il processo che riceve il segnale
	signal(SIGUSR1, handler);
}


int main(int argc, char **argv){
	int  pidFiglio, status,ritorno,ret,fd,L, *pid;
  int q,Q, occ;
	char ch, ok='k';
	char linea[255];
	typedef int pipe_t[2];
	signal(SIGUSR1, handler);


	//CONTROLLO PARAMETRI
	if(argc < 5){
		printf("Errore nel numero di parametri\n");
		exit(1);
	}
	Q = argc-3; //numero dei processi figli


	L=atoi(argv[2]);
	if(L<0){
		puts("Errore numero negativo");
		exit(2);
	}

	//ARRAY FIGLI
	pid=malloc(sizeof(int)*Q);
	if(pid==NULL){
		printf("Errore allocazione array pid\n");
		exit(3);
	}

	//CICLO FIGLI
	for(int q=0; q < Q; q++){

		if ((pid[q] = fork()) < 0){
			printf("Errore creazione figlio\n");
			exit(4);
		}


	//FIGLIO C-ESIMO
	if(pid[q]== 0){
		//SCHEMA DI COMUNICAZIONE: ogni figlio manda al padre la lunghezza l e la linea, per ogni linea del file


		//apertura file associato
		if((fd=open(argv[1],O_RDONLY))<0){
			puts("Errore apertura file");
			exit(-1);
		}

	//CONTA OCCORRENZE + STAMPA OUTPUT
  occ=0;
	while(read(fd,&ch, 1)){	//leggo il file carattere per carattere
    if(ch==argv[3+q][0])
      occ++; 	//incremento il contatore di occorrenze se il carattere corrisponde con la lettera associata
		//QUANDO ARRIVO ALLA FINE DELLA LINEA
		if(ch=='\n'){
			pause(); //aspetta segnale dal padre per poter scrivere
      printf("%d occorrenze del carattere '%c'\n",occ,argv[3+q][0]);
			sleep(1);
			kill(getppid(),SIGUSR1);	//manda ok al padre
      ritorno=occ; //salvo il valore da tornare nella exit
			occ=0; //azzero il contatore per la prossima riga
		}
	}

		exit(ritorno); //torno il numero di occorrenze dell'ultima linea
	}
}


//PADRE
	printf("Padre %d sta per iniziare a leggere i messaggi della pipe \n", getpid());

	//manda ok dopo aver stampato il numero di linea, riceve ok prima di dare l'ok al prossimo figlio
  for(int i=1; i<=L; i++){
    printf("Linea %d:\n", i);
  	for(q=0; q<Q; q++){
			sleep(1);
  		kill(pid[q],SIGUSR1);
			pause();
  	}
  }


//ASPETTA TUTTI I FIGLI
	for(int q=0; q<Q; q++){
        pidFiglio = wait(&status);
        if (pidFiglio < 0){
            printf("Errore wait\n");
            exit(5);
        }

        if ((status & 0xFF) != 0)
            printf("Figlio con pid %d terminato in modo anomalo\n", pidFiglio);
        else{
            ret=(int)((status >> 8) & 0xFF);
            printf("Il figlio con pid=%d ha ritornato %d (se 255 problemi!)\n", pidFiglio,ret);
        }
    }
		exit (0);
}
