#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <time.h>
#define PERM 0664

typedef int pipe_t[2];

int main(int argc, char **argv){
	srand(time(NULL));
	int *pid, pidFiglio, status, ritorno, fd, ret; //gestione comunicazione padre e valori di ritorno
	char ch;
	char *buff, *F;
  int B, L; //num di figli e num di linee
  int dim; //dimensione dei blocchi


	//CONTROLLO STRETTO SUL MUMERO DI PARAMETRI
	if(argc != 4){
		printf("Errore nel numero di parametri\n");
		exit(1);
	}
  //controllo secondo parametro
  B=atoi(argv[2]);
  if(B<0){
    puts("Errore secondo parametro negativo\n");
    exit(2);
  }
  //controllo terzo parametro
  L=atoi(argv[3]);
  if(L<0){
    puts("Errore terzo parametro negativo\n");
    exit(3);
  }
  dim=L/B; //calcolo dimensione del blocco (uguale per tutti, si suppone che L sia divisibile per B)

  //CREAZIONE FILE
  F=malloc(sizeof(argv[1])+8);
  if(F==NULL){
    printf("errore malloc\n");
    exit(4);
  }
  strcpy(F, argv[1]);
  strcat(F, ".Chiara");
	//CREAZIONE FILE
	int fdTemp=open("Camilla", O_WRONLY | O_CREAT | O_TRUNC ,PERM);
  if(fdTemp<0){
    printf("Errore creazione file\n");
    exit(5);
  }
  else
    printf("creato file %s\n", F);


  //ALLOCAZIONE VETTORE PID FILGI
  pid=malloc(sizeof(int)*(B));
  if(pid==NULL){
      printf("Errore allocazione vettore di pid\n");
      exit(6);
  }
	//ALLOCAZIONE Q PIPE
	pipe_t *piped=malloc(sizeof(pipe_t)*(B));
  if(piped==NULL){
    printf("Errore allocazione vettore di pipe\n");
    exit(7);
  }
  //CREAZIONE Q PIPE
  for(int i=0; i < B; i++){
    if (pipe (piped[i]) < 0){
      printf("Errore creazione\n");
      exit(8);
    }
  }

	//INIZIO FOR FIGLI
	for(int q=0; q < B; q++){
		//CREAZIONE B FIGLI
		if ((pid[q] = fork()) < 0){
			printf("Errore creazione figlio\n");
			exit(9);
		}

	//CODICE FIGLIO q-ESIMO
	if(pid[q] == 0){
    //i figli tornano -1 in caso di errore (che verrà letto come 255 dal padre)
    //SCHEMA DI COMUNICAZIONE: i figli mandano al padre il primo carattere
		//CHIUSURA PIPE INUTILIZZATE
		for (int k=0; k<B; k++){
			close (piped [k][0]);
			if(k!=q){
				close(piped [k][1]);
			}
		}
		//APERTURA FILE
		if((fd=open(argv[1], O_RDONLY)) < 0){
			printf("Errore in apertura file %s\n", argv[q+1]);
							exit(-1);
		}

		// LETTURA FILE
    lseek(fd, dim*q, SEEK_SET); //mi sposto akll'inizio della sezione da leggere
    buff=malloc(dim);
    if(buff==NULL){
      printf("Errore allocazione buffer del blocco (figlio)\n");
      exit(-1);
    }
    ret=read(fd, buff, dim); //leggo un blocco del FILE (e faccio controllo)
    if(ret!=dim){
      printf("Errore lettura blocco da file");
      exit(-1);
    }
    //scrivo al padre l'ultimo carattere
    ch=buff[dim-1];
    ret=write(piped[q][1], &ch, 1);
    if(ret!=1){
      printf("Errore scrittura carattere al padre");
      exit(-1);
    }

		exit(dim); //ritorno la dimensione del blocco
	 }
  }
  //FINE FOR FIGLI

	//CODICE PADRE
	//chiude tutte le pipe inutilizzate
	for(int k=0 ; k<B; k++){
		close (piped[k][1]);
	}

	printf("Padre %d sta per iniziare a leggere i messaggi della pipe \n", getpid());
	//il for garantisce di rispettare l'ordine dei file. scorriamo tutti i processi figli
	for(int q=0; q<B; q++) {
			//legge l'ultimo carattere del blocco di ciascun figlio
			ret=read (piped[q][0], &ch, 1);
      if(ret!=1){
        printf("Errore lettura carattere (padre)\n");
        exit(10);
      }
			//STAMPO PID, INDICE FIGLIO e ULTIMO CARATTERE DEL BLOCCO
			printf("PID: %d, INDICE: %d, CARATTERE: %c\n",pid[q], q, ch);

	}

	//ASPETTA LA TERMINAZIONE DI TUTTI I FIGLI E STAMPA I RELATIVI VALORI DI USCITA
	for(int c=0; c<B; c++){
        pidFiglio = wait(&status);
        if (pidFiglio < 0){
            printf("Errore wait\n");
            exit(11);
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
