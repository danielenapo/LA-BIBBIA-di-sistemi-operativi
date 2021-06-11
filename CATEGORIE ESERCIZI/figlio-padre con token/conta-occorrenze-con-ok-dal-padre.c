#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>

typedef int pipe_t[2];

int main(int argc, char **argv){
	int  pidFiglio, status,ritorno,ret,fd,L,pid;
  int q,Q, occ;
	char ch, ok='k';
	char linea[255];


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

	//CREAZIONE N PIPE PADRE-FIGLIO
	pipe_t *piped=malloc(sizeof(pipe_t)*(Q));
	if(piped==NULL){
		puts("Errore allocazione");
		exit(3);
	}
  //CREAZIONE N PIPE FIGLIO-PADRE
  pipe_t *pipedF=malloc(sizeof(pipe_t)*(Q));
  if(pipedF==NULL){
    puts("Errore allocazione");
    exit(4);
  }

	//CICLO FIGLI
	for(int q=0; q < Q; q++){
		if (pipe (piped[q]) < 0){
			printf("Errore creazione pipe\n");
			exit(5);
		}
    if (pipe (pipedF[q]) < 0){
      printf("Errore creazione pipe\n");
      exit(6);
    }


		if ((pid = fork()) < 0){
			printf("Errore creazione figlio\n");
			exit(7);
		}


	//FIGLIO C-ESIMO
	if(pid== 0){
		//SCHEMA DI COMUNICAZIONE: ogni figlio manda al padre la lunghezza l e la linea, per ogni linea del file

		//chiudo pipe inutilizzate. ogni figlio non legge da nessuna pipe, scrive solo su piped[i] (comunicando col padre)
		for (int j=0; j<Q; j++){
				close(piped [j][1]);
        close(pipedF[j][0]);
			if(j!=q){
				close(piped [j][0]);
        close(pipedF [j][1]);
      }
    }

		//apertura file associato
		if((fd=open(argv[1],O_RDONLY))<0){
			puts("Errore apertura file");
			exit(-1);
		}

	//CONTA OCCORRENZE + SCRITTURA OUTPUT
  occ=0;
	while(read(fd,&ch, 1)){  //leggo un carattere alla volta dal file
    if(ch==argv[3+q][0])  //incremento il contatore di occorrenze se il carattere letto corrisponde al carattere da cercare
      occ++;
		//QUANDO ARRIVO ALLA FINE DELLA LINEA
		if(ch=='\n'){
      ret=read(piped[q][0],&ok, 1); //aspetta l'ok del padre
      if(ret!=1){
        printf("Errore lettura figlio\n");
        exit(8);
      }
			//se questa era la linea che dovevo salvare, finisco il ciclo e la scrivo al padre
      printf("%d occorrenze del carattere '%c'\n",occ,argv[3+q][0]);
      ret=write(pipedF[q][1],&ok,1); //scrivo l'ok al padre
      if(ret!=1){
        printf("Errore scrittura padre\n");
        exit(9);
      }
      ritorno=occ;  //salvo il valore da tornare
			occ=0;
		}
	}

		exit(ritorno); //torno il numero di occorrenze dell'ultima linea letta
	}
}


//PADRE
	printf("Padre %d sta per iniziare a leggere i messaggi della pipe \n", getpid());

	//chiusura pipe inutilizzate
	for(int k=0 ; k<Q; k++){
		close (piped[k][0]);
    close(pipedF[k][1]);
	}

	//legge linea per ogni figlio (fa esattamente n letture)
  for(int i=1; i<=L; i++){
    printf("Linea %d:\n", i);
  	for(q=0; q<Q; q++){
  		ret=write(piped[q][1],&ok,1);
      if(ret!=1){
        printf("Errore scrittura a figlio\n");
        exit(10);
      }
      ret=read(pipedF[q][0],&ok,1); //scrivo l'ok al padre
      if(ret!=1){
        printf("Errore lettura padre\n");
        exit(11);
      }
  	}
  }


//ASPETTA TUTTI I FIGLI
	for(int q=0; q<Q; c++){
        pidFiglio = wait(&status);
        if (pidFiglio < 0){
            printf("Errore wait\n");
            exit(12);
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
