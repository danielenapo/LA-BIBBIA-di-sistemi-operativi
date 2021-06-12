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
	long int c1; //max occorrenze
	int c2; //indice del processo che ha calcolato il massimo
  long int c3; //somma di tutte le occorrenze
}strut;

typedef int pipe_t[2];


int main(int argc, char **argv){
	int  pidFiglio, status,ret,fd;
  int *pid, n;
	char Cx,inpbuf;
	long int occ0=0;


	//CONTROLLO PARAMETRI
	if(argc < 4){
		printf("Errore nel numero di parametri\n");
		exit(1);
	}
	n = argc-2; //numero dei processi figli

  //controllo ultimo parametro sia singolo carattere
	if(argv[argc-1][1]!=0){
		puts("Errore ultimo parametro non singolo carattere\n");
		exit(2);
	}
  Cx=argv[argc-1][0]; //assegnazione ultimo parametro
  printf("Carattere %c, numero di figli: %d\n", Cx, n);

  //ALLOCAZIONE ARRAY pid
  pid=malloc(sizeof(int)*n);
  if(pid==NULL){
    puts("Errore allocazione array pid\n");
    exit(3);
  }

	//ALLOCAZIONE e CREAZIONE  N PIPE
	pipe_t *piped=malloc(sizeof(pipe_t)*(n));
	if(piped==NULL){
		puts("Errore allocazione array pipe\n");
		exit(4);
	}
  for(int c=0; c < n; c++){
    if (pipe (piped[c]) < 0){
      printf("Errore creazione pipe\n");
      exit(5);
    }
  }

	//CICLO FIGLI
	for(int c=0; c < n; c++){
    //fork filgio c-esimo
		if ((pid[c] = fork()) < 0){
			printf("Errore creazione figlio\n");
			exit(6);
		}

	//CODICE FIGLIO C-ESIMO
	if(pid[c] == 0){
    //il figlio torna -1 in caso di errore (che verrà letto come 255 dal padre), il suo indice in caso di successo (si suppone essere <255)
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
		S[c].c1=-1;
		S[c].c2=0;
    S[c].c3=0;
	//SE NON E' IL PRIMO FIGLIO, LEGGO IN PIPE DA QUELLO PRECEDENTE
		if(c!=0){
			ret=read(piped[c-1][0],S,sizeof(strut)*c);
			if(ret!=sizeof(strut)*c){
				puts("Errore lettura pipe figlio");
				exit(-1);
      }
		}

		//apertura file associato
		if((fd=open(argv[c+1],O_RDONLY))<0){
			puts("Errore apertura file");
			exit(-1);
		}
    //conta occorrenze
    occ0=0;
    while(read(fd,&inpbuf,1)!=0){
      if(inpbuf==Cx)
        occ0++;
    }
    S[c].c3=occ0; //pongo c3=num di occorrenze nel file associato c-esimo

    //trova il massimo delle occorrenze (e lo salva in c1, poi salva in c2 l'indice)
    if(c!=0){
      for(int i=0; i<c; i++){
        if(S[c].c1<=S[i].c3){
          S[c].c1=S[i].c3; //salvo il valore del max
          S[c].c2=i; //salvo l'indice del max
        }
      }
    }
    else
      S[c].c1=occ0; //se è il primo figlio, come max metto sè stesso

		//TUTTI I FIGLI SCRIVONO IN PIPE AL SUCCESSIVO IL PROPRIO ARRAY DI STRUCT (l'ultimo manderà al padre)
		ret=write(piped[c][1],S,sizeof(strut)*(c+1));
    if(ret!=sizeof(strut)*(c+1)){
      puts("Errore scrittura a successivo (figlio)");
      exit(-1);
    }

		exit(c); //torno l'indice
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
		exit(7);
	}

	//legge l'array di strutture dell'ultimo FIGLIO
	ret=read(piped[n-1][0],S,sizeof(strut)*n);
	if(ret!=sizeof(strut)*n){
		puts("Errore lettura pipe padre");
		exit(8);
	}

	//stampa contenuto struct per ogni figlio
	//for(int i=0; i<n; i++)
	printf("INDICE MAX: %d, PID: %d, FILE: %s, OCCORRENZE MAX: %ld\n", S[n-1].c2, pid[S[n-1].c2], argv[(S[n-1].c2)+1],S[n-1].c1);


//ASPETTA TUTTI I FIGLI
	for(int c=0; c<n; c++){
        pidFiglio = wait(&status);
        if (pidFiglio < 0){
            printf("Errore wait\n");
            exit(9);
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
