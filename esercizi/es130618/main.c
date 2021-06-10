#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>

typedef struct{
	int c1; //pid
	int c2; //lunghezza linea (compreso terminatore)
}strut;

void bubbleSort(strut v[], int dim){
	 int i;
	 strut temp;
	  bool ordinato = false;
		while (dim>1 && !ordinato) {
			  ordinato = true; /* hp: è ordinato */
				for (i=0; i<dim-1; i++)
				if (v[i].c2>v[i+1].c2) {
					temp=v[i];
					v[i]=v[i+1];
					v[i+1]=temp;

					ordinato = false;
				}
					dim--;
			}
	}


typedef int pipe_t[2];

int main(int argc, char **argv){
  //DICHIAARAZIONI
	int  pidFiglio, status,ret,fd, N;
	char inpbuf;
	int y; //lunghezza in linee del file (passato come parametro)
  int l; //lunghezza linea

	//CONTROLLO PARAMETRI
	if(argc < 4){
		printf("Errore nel numero di parametri\n");
		exit(1);
	}
	N = argc-2; //numero dei processi figli
	y=atoi(argv[argc-1]);
  //controlll su y
	if(y<0){
		puts("Errore y minore di 0");
		exit(2);
	}
  //ALLOCAZIONE ARRAY PID FIGLI
  int *pid=malloc(sizeof(int)*N); //vettore di pid per salvarmi i pid dei figli
  if(pid==NULL){
		puts("Errore allocazione");
		exit(3);
	}
	//ALLOCAZIONE N PIPE
	pipe_t *piped=malloc(sizeof(pipe_t)*(N));
	if(piped==NULL){
		puts("Errore allocazione");
		exit(4);
	}


	//CICLO FIGLI
	for(int c=0; c < N; c++){
    //Creazione pipe
		if (pipe (piped[c]) < 0){
			printf("Errore creazione pipe\n");
			exit(5);
		}
    //fork figli
		if ((pid[c] = fork()) < 0){
			printf("Errore creazione figlio\n");
			exit(6);
		}


	//CODICE FIGLIO C-ESIMO
	if(pid[c] == 0){
		//SCHEMA DI COMUNICAZIONE: ogni figlio manda al successivo il suo array di strut

		//chiudo pipe inutilizzate (ossia tutte quelle di lettura tranne quella precedente, tutte quelle di scrittura tranne quella corrente)
		for (int k=0; k<N; k++){
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

		//CONTA OCCORRENZE DAL FILE
		//apertura file associato
		if((fd=open(argv[c+1],O_RDONLY))<0){
			puts("Errore apertura file");
			exit(-1);
		}
		//conta lunghezza linea
    l=0; //azzero contatore linea
		while(read(fd,&inpbuf,1)){
      l++;
			if(inpbuf=='\n'){
        //SE NON E' IL PRIMO FIGLIO, LEGGO IN PIPE DA QUELLO PRECEDENTE
        if(c!=0){
          ret=read(piped[c-1][0],S,sizeof(strut)*c);
          if(ret!=sizeof(strut)*c){
            puts("Errore lettura pipe figlio");
            exit(-1);
          }
        }
        //salvo i valori trovati
        S[c].c2=l;
        //TUTTI I FIGLI SCRIVONO IN PIPE AL SUCCESSIVO IL PROPRIO ARRAY DI STRUCT (l'ultimo manderà al padre)
        ret=write(piped[c][1],S,sizeof(strut)*(c+1));
        if(ret!=sizeof(strut)*(c+1)){
          puts("Errore scrittura pipe figlio");
          exit(-1);
        }
        l=0;//azzero contatore per la prossima linea
      }


		}


		exit(c); //torno l'indice
	}
}


//PADRE
	printf("Padre %d sta per iniziare a leggere i messaggi della pipe \n", getpid());

	//chiusura pipe inutilizzate (tutte quelle di scrittura, tutte quelle di lettura tranne l'ultima)
	for(int k=0 ; k<N; k++){
		close (piped[k][1]);
		if(k!=(N-1))
			close (piped[k][0]);
	}

	//array di struttura, con dimensione n
	strut *S=malloc(sizeof(strut)*N);
	if(S==NULL){
		puts("Errore allocazione");
		exit(-1);
	}


	//legge l'array di strutture dell'ultimo figlio, per ogni riga del FILE
  for(int j=0; j<y; j++){
  	ret=read(piped[N-1][0],S,sizeof(strut)*N);
  	if(ret!=sizeof(strut)*N){
  		puts("Errore lettura pipe padre");
  		exit(7);
  	}
    //ordina i figli in base alla lunghezza della linea
    bubbleSort(S, N);
  	//stampa contenuto struct per ogni figlio
  	for(int i=0; i<N; i++){
      for(int k=0; k<N; k++){
        if(S[i].c1==pid[k])
          printf("PID: %d, FILE: %s, LINEA: %d, LUNGHEZZA: %d\n", S[i].c1, argv[k+1], j+1,S[i].c2);
      }
    }
  }


//ASPETTA TUTTI I FIGLI
	for(int c=0; c<N; c++){
        pidFiglio = wait(&status);
        if (pidFiglio < 0){
            printf("Errore wait\n");
            exit(8);
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
