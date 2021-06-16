#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#define PERM 0644


typedef int pipe_t[2];
typedef char linea[250]; //definisco un tipo array di char di max 250 caratteri per salvare le linee


int main (int argc, char **argv){
	int Z; 			//numero di file/processi
	int pid;		//pid per fork
	pipe_t *pipes;		/* array di pipe usate a ring da primo figlio, a secondo figlio .... ultimo figlio e poi a primo figlio: ogni processo legge dalla pipe q e scrive sulla pipe (q+1)%Q */
	int z,j; 		/* indici */
	int fd; 		/* file descriptor */
	int pidFiglio, status, ritorno;	/* per valore di ritorno figli */
	char inpbuf;	  //buffer del carattere letto dal file
	int nr,nw;              /* variabili per salvare valori di ritorno di read/write da/su pipe */
  linea BUFF; //buffer su cui salverò la linea corrente
  linea *All_l; //array con tutte le linee lette dai figli
  int fdCreato; //file descriptor del file creato

	/* controllo sul numero di parametri almeno 2 file */
	if (argc < 3){
		printf("Errore numero di parametri\n");
		exit(1);
	}
	Z = argc-1; //calcolo del numero di processi/file
  printf("Numero di processi da creare %d\n", Z); //stampa di debug

  //allocazione array All_l
  if ((All_l=malloc(Z*sizeof(linea))) == NULL){
    printf("Errore allocazione array di linee\n");
    exit(2);
  }

  //CREAZIONE FILE
  if((fdCreato=open("Napolitano",O_CREAT | O_WRONLY, PERM))<0){ //il nome del file deve corrispondere al mio cognome
    puts("Errore creazione file\n");
    exit(3);
  }


	// allocazione pipe
	if ((pipes=(pipe_t *)malloc(Z*sizeof(pipe_t))) == NULL){
		printf("Errore allocazione pipe\n");
		exit(4);
	}
	/* creazione pipe */
	for (z=0;z<Z;z++){
		if(pipe(pipes[z])<0){
			printf("Errore creazione pipe\n");
			exit(5);
		}
	}

	// CICLO DEI FIGLI
	for (z=0;z<Z;z++){
    //fork del figlio
		if ((pid=fork())<0)
		{
			printf("Errore creazione figlio\n");
			exit(6);
		}

    // codice figlio
		if (pid == 0){
      //i figli tornano -1 in caso di errore
      //SCHEMA DI COMUNICAZIONE: ring

  		/* chiusura pipes inutilizzate */
  		for (j=0;j<Z;j++)
  		{	/* si veda commento nella definizione dell'array pipes per comprendere le chiusure */
  			if (j!=z)
  				close (pipes[j][0]);
  			if (j != (z+1)%Z) //uso il modulo per collegare l'ultimo figlio al primo
  				close (pipes[j][1]);
  		}

  		// apertura file associato
  		if ((fd=open(argv[z+1],O_RDONLY))<0)
  		{
  			printf("Impossibile aprire il file %s\n", argv[z+1]);
  			exit(-1);
  		}

  		/* inizializziamo l'indice dei caratteri letti per ogni singola linea */
      j=0;
  		/* con un ciclo leggiamo tutti i caratteri */
  	  while(read(fd,&inpbuf,1)){
        BUFF[j]=inpbuf;
        if(inpbuf=='\n'){
    			nr=read(pipes[z][0],All_l ,Z*sizeof(linea)); //legge l'array dal precedente
    			if (nr != Z*sizeof(linea)) //controlliamo il risultato della lettura da pipe
    			{
    				printf("Figlio %d ha letto un numero di byte sbagliati %d\n", z, nr);
    				exit(-1);
    			}
          BUFF[j+1]=0; //dopo il '\n' metto il terminatore di stringa per poter effettuare la strcpy senza problemi
          //salvo nella posizione z-esima di All_l la linea letta(contenuta in BUFF)
          strcpy(All_l[z], BUFF);
          //se è il figlio Z-1-esimo, scrivo sul file creato il vettore
          if(z==(Z-1)){
            nw=write(fdCreato, All_l, Z*sizeof(linea));
            if (nw != Z*sizeof(linea)) //controllo scrittura corretta
            {
              printf("L'ultimo figlio ha scritto sul file creato un numero di byte sbagliati\n");
              exit(-1);
            }
          }

          //TUTTI I FIGLI SCRIVONO AL SUCCESSIVO L'ARRAY
          nw=write(pipes[(z+1)%Z][1],All_l,Z*sizeof(linea)); //uso il modulo in modo che l'ultimo figlio scrivi al primo
    			if (nw != Z*sizeof(linea))
    			{
    				printf("Figlio %d ha scritto un numero di byte sbagliati al successivo %d\n", z, nr);
    				exit(-1);
    			}
          ritorno=j; //mi salvo il numero di caratteri, perchè è il valore da ritornare
          j=0; //azzero il contatore di caratteri per la prossima linea

        }
        else
          j++; //se non c'è il terminatore, incremento il contatore di caratteri

  	  }

  		/* ogni figlio deve tornare il numero di caratteri dell'ultima linea (compreso il terminatore)*/
  		exit(ritorno);
		}
	} //FINE FOR FIGLI

	//CODICE PADRE
	// chiusura di tutte le pipe che non usa, a parte la prima perche' il padre deve dare il primo OK al primo figlio.
	for(z=1;z<Z;z++){ /* l'indice lo facciamo partire quindi da 1! (l'unica pipe che ci serve è quella del primo figlio)*/
		close (pipes[z][0]);
		close (pipes[z][1]);
	}

	// mando vettore al primo figlio, per iniziare il ring. controllo anche se è avvenuta correttamente. il vontenuto non ha importanza
	nw=write(pipes[0][1],All_l,Z*sizeof(linea));
	if (nw != Z*sizeof(linea)){
		printf("Padre ha scritto un numero di byte sbagliati al primo figlio %d\n", nw);
	  exit(7);
	}
	/* ora possiamo chiudere anche il lato di scrittura, ma non quello di lettura  (altrimenti va in SIGPIPE)*/
	close(pipes[0][1]);


	/* Il padre aspetta i figli */
	for (z=0; z < Z; z++){
	  pidFiglio = wait(&status);
	  if (pidFiglio < 0)
	  {
	          printf("Errore in wait\n");
	          exit(8);
	  }
	  if ((status & 0xFF) != 0)
	          printf("Figlio con pid %d terminato in modo anomalo\n", pidFiglio);
	  else
	  {
	ritorno=(int)((status >> 8) & 0xFF);
	  	printf("Il figlio con pid=%d ha ritornato %d (se 255 problemi)\n", pidFiglio, ritorno);
	  }
	}
	exit(0);
}
