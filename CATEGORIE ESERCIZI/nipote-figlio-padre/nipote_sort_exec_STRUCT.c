#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <stdbool.h>

typedef struct{
	int c1; //pid nipote
	int c2; //lunghezza linea (con terminatore)
	char linea[250]; //contenuto linea corrente
}strut;

typedef int pipe_t[2];

int main(int argc, char **argv){
	int i=0, pidFiglio, ppid, status, ritorno, ret; //variabili per comunicazioni pipe e gestione del valore di ritorno dei figli
	int n; //numero dei processi figli
	strut S; //variabile della struttura

	//CONTROLLO PARAMETRI
	if(argc < 4){
		printf("Errore nel numero di parametri\n");
		exit(1);
	}
  n=argc-1; //calcolo del numero di figli/file

	int *pid=malloc(sizeof(int)*n); //vettore di pid per salvarmi i pid dei figli
  if(pid==NULL){
    printf("errore allocazione vettore pid\n");
    exit(2);
  }

	//CREAZIONE N PIPE
	pipe_t *piped=malloc(sizeof(pipe_t)*(n));
  if(pid==NULL){
    printf("errore allocazione vettore pipe\n");
    exit(3);
  }
  for(int c=0; c < n; c++){
    if (pipe (piped[c]) < 0){
      printf("Errore creazione pipe\n");
      exit(4);
    }
  }


  //INIZIO CICLO FIGLI
	for(int c=0; c < n; c++){
    //fork figli
		if ((pid[c] = fork()) < 0){
			printf("Errore creazione figlio\n");
			exit(5);
		}

	//CODICE FIGLIO C-ESIMO
	if(pid[c] == 0){
    //i figli tornano -1 in caso di errore (interpetrato come 255 dal padre)
    //SCHEMA DI COMUNICAZIONE:
    //il nipote esegue exec di sort, mandado il risultato al figlio su pipedN.
    //il figlio legge da pipedN, crea la struttura con i dati ricevuti e la manda al padre con piped

		//CHIUDO PIPE INUTILIZZATE
		for (int k=0; k<n; k++){
			close (piped [k][0]); //chiudo tutti piped in lettura
			if(k!=c)
				close(piped [k][1]); //chiudo tutti piped in scrittura, tranne quello dell'indice corrente
		}

		//creazione pipe NIPOTE
		pipe_t pipedN;
		if (pipe (pipedN) < 0){
			printf("Errore creazione pipe nipote");
			exit(-1);
		}

    //fork nipote
		if ((ppid = fork()) < 0){
			printf("Errore creazione nipote");
			exit(-1);
		}

    //CODICE NIPOTE
		if(ppid==0){
			close(piped[c][1]);
			close(1);
			dup(pipedN[1]); //ridirezione pipedN su stdout
			close(pipedN[0]); //non mi serve la lettura
			close(pipedN[1]);  //la pipe di scrittura ora è in stdout, quindi tolgo pipedN

			//Esecuzione sort (che scrive in pipe)
			execlp("sort","sort", "-f", argv[c+1],(char *) 0); //opzione -f ingora i case
			perror("Errore comando sort");
			exit(-1);
		}

		//FIGLIO
		S.c1=ppid; //in c1 metto il pid del nipote, salvato in fork
		close(pipedN[1]); //deve leggere, chiudo pipedN in scrittura

		while(read(pipedN[0],&(S.linea[i]),1)){
			if(S.linea[i]=='\n'){
					S.c2=i+1; //comprende il terminatore
					ret=write(piped[c][1],&S,sizeof(S));//scrive al padre la linea
          if(ret!=sizeof(S)){
            perror("Errore write al padre");
            exit(-1);
          }
          break;//esco dal ciclo, perchè serve solo la prima riga
			}
			else
				i++;
		}
		exit(i); //ritorno la lunghezza della linea (senza terminatore)
	}
}
//FINE FOR


//PADRE
	printf("Padre %d sta per iniziare a leggere i messaggi della pipe \n", getpid());

  //chiudo pipe inutilizzate (tutte quelle in scrittura su piped)
	for(int k=0 ; k<n; k++){
		close (piped[k][1]);
	}

  //scorro tutti i figli, leggendo per ognuno la sua struct e stampandola in output
	for(i=0; i<n; i++) {
		ret=read(piped[i][0], &S, sizeof(S));	//legge dalla pipe la struct (ed eseguo controllo lettura)
    if(ret != sizeof(S)){
      puts("Errore lettura struct\n");
      exit(6);
    }
    S.linea[S.c2 -1]=0; //rendo la linea una stringa, mettendo il terminatore '\0' usando come indice c2

    //stampa su output: stampo tutto il contenuto della struct e il nome del file associato
		printf("PID NIPOTE: %d FILE: %s LUNGHEZZA: %d CONTENUTO: %s\n",S.c1, argv[i+1], S.c2, S.linea);
	}

  //ASPETTA TUTTI I FIGLI
	for(int c=0; c<n; c++){
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
