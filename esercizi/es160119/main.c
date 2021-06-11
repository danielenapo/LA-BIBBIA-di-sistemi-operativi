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
	int pidFiglio, status, ritorno,pid, fd; //gestione processi e valori di ritorno
	int n, j, numLinee, ret;
  int f1, f2; //lunghezze in caratteri delle linee dei due file
	char inpbuf,ok; //ok è il token
  char linea[255]; //si suppone essere minore di 255 caratteri
  bool finito=false;  //var per capire se i figli hanno finito di leggere tutte le linee

	//CONTROLLO STRETTO PARAMETRI (devono esserci due parametri)
	if(argc != 3){
		printf("Errore nel numero di parametri\n");
		exit(1);
	}
	n= argc-1; //numero dei processi figli/file, ossia 2

	//allocazione N PIPE DATI
	pipe_t *piped=malloc(sizeof(pipe_t)*(n));
	if(piped==NULL){
		puts("Errore allocazione array di pipe dati");
		exit(2);
	}
	//allocazione n pipe token
	pipe_t *pipex=malloc(sizeof(pipe_t)*n);
	if(pipex==NULL){
		puts("Errore allocazione array di pipe token");
		exit(3);
	}

  //CREAZIONE PIPE
  for(int c=0; c < n; c++){
    //pipe dati
    if (pipe (piped[c]) < 0){
      printf("Errore creazione\n");
      exit(4);
    }
    //pipe token
    if (pipe (pipex[c]) < 0){
      printf("Errore creazione\n");
      exit(5);
    }
  }

	//CICLO FIGLI
	for(int c=0; c < n; c++){
		//fork figlio
		if ((pid=fork()) < 0){
			printf("Errore creazione figlio\n");
			exit(6);
		}

	//CODICE FIGLIO C-ESIMO
	if(pid == 0){
    //il figlio ritorna -1 in caso di errore
		//SCHEMA DI COMUNICAZIONE:
		//il figlio legge la riga, manda la sua lunghezza al padre, legge il token dal padre e stampa l'output solo se questo contiene il carattere 's' (ossia se ha la stessa lunghezza dell'altro figlio)

		//chiudo pipe inutilizzate (uso piped per scrittura del carattere e pipex per lettura del token)
		for (int k=0; k<n; k++){
			close (piped [k][0]);	//chiudo tutte le pipe dati in lettura
			close(pipex[k][1]);	//chiudo tutte le pipe di token in scrittura
			if(k!=c){
				close(piped [k][1]);	//chiudo tutte le pipe dati in scrittura tranne quella corrente
				close(pipex[k][0]);	//chiudo tutte le pipe di token in lettura tranne quella corrente
			}
		}

		//APERTURA FILE
		if((fd=open(argv[c+1], O_RDONLY)) < 0){
			printf("Errore in apertura file %s\n", argv[c+1]);
			exit(-1);
		}

		//LETTURA FILE
		j=0;//contatore lunghezza linea
		numLinee=0; //contatore delle linee (uguale per i due file)
		while(read(fd, &inpbuf, 1)){ //leggo un carattere alla volta
			//se è il primo carattere lo salvo in una variabile che manderò al padre  a fine riga
      linea[j]=inpbuf;
			//se arrivo alla fine della linea scrivo al padre il primo carattere, leggo il token e in base al suo valore scrivo in output o meno
			if(inpbuf=='\n'){
        linea[j]=0;//la rendo una stringa
        j++;//incremento j per includere anche il terminatore
        ret=write(piped[c][1],&j, sizeof(int)); //mando al padre la lunghezza della LINEA
        if(ret!=sizeof(int)){
          perror("Errore scritturalunghezza linea al padre\n");
          exit(-1);
        }
        ret=read(pipex[c][0],&ok,1); //leggo il token del padre
        if(ret!=1){
          perror("Errore lettura token figlio\n");
          exit(-1);
        }

        if(ok=='s'){ //se vale 's', vuol dire che ha la stessa lunghezza dell'altro figlio, e che può mandare la riga al padre
          ret=write(piped[c][1],linea,j);
          if(ret!=j){
            perror("Errore scrittura linea al padre\n");
            exit(-1);
          }
        }
				numLinee++; //incremento il contatore di linee (da tornare al padre alla fine)
				j=0;//azzero la lunghezza della linea per la prossima

			}
			else
				j++; //se è un carattere della linea, incemento il contatore
		}

		exit(numLinee); //ritorno il numero di linee
	}
} //fine ciclo figli



//PADRE
	//chiude le piped inutilizzate
	for(int k=0 ; k<n; k++){
		close (piped[k][1]);
		close(pipex[k][0]);
	}

	printf("Padre %d sta per iniziare a leggere i messaggi della pipe \n", getpid());

  j=1;//contatore delle linee
	while(finito==false){
		finito=true; //suppongo che i figli abbiano finito di leggere
		ret=read(piped[0][0],&f1,sizeof(int)); //legge la lunghezza di entrambi i figli
    ret=read(piped[1][0],&f2,sizeof(int));
    if(ret==sizeof(int)){ //controllo che la lettura sia andata a buon fine (se è diverso significa che i figli sono finiti)
      finito=false;//significa che i figli non hanno ancora finito di leggere, quindi posso continuare col figlio
      //se sono lunghi uguali scrivo un segnale positivo e leggo la linea, scrivendola in output, altrimenti mando token negativo e stampo che è stata modificata
      if(f1==f2){
        ok='s';
        ret=write(pipex[0][1],&ok,1);
        if(ret!=1){
          perror("Errore scrittura token al primo figlio\n");
          exit(7);
        }
        ret=write(pipex[1][1],&ok,1);
        if(ret!=1){
          perror("Errore scrittura token al secondo figlio\n");
          exit(8);
        }
        //LETTURA E SCRITTURA IN OUTPUT DELLE LINEE
        ret=read(piped[0][0],linea,f1);
        if(ret!=f1){
          perror("Errore lettura linea del primo figlio\n");
          exit(9);
        }
        printf("linea %d file %s: %s\n", j, argv[1], linea);
        ret=read(piped[1][0],linea,f2);
        if(ret!=f2){
          perror("Errore lettura linea del secondo figlio\n");
          exit(10);
        }
        printf("linea %d file %s: %s\n", j, argv[2], linea);
      }
      else{
        ok='n';
        ret=write(pipex[0][1],&ok,1);
        if(ret!=1){
          perror("Errore scrittura token al primo figlio\n");
          exit(11);
        }
        ret=write(pipex[1][1],&ok,1);
        if(ret!=1){
          perror("Errore scrittura token al secondo figlio\n");
          exit(12);
        }
        printf("linea %d: è stata modificata\n", j);
      }
      j++;
    }
	}



//PADRE ASPETTA TUTTI I FIGLI e stampa i valori tornati (e segnala eventuali errori o terminazioni insapettate)
	for(int c=0; c<n; c++){
        pidFiglio = wait(&status);
        if (pidFiglio < 0){
            printf("Errore wait\n");
            exit(13);
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
