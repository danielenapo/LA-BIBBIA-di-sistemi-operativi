#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>

typedef int pipe_t[2];

int main (int argc, char **argv){
	int Q; 			//numero di file/processi
	int pid;		//pid per fork
	pipe_t *pipes;		/* array di pipe usate a ring da primo figlio, a secondo figlio .... ultimo figlio e poi a primo figlio: ogni processo legge dalla pipe q e scrive sulla pipe (q+1)%Q */
	int q,j; 		/* indici */
	int fd; 		/* file descriptor */
	int pidFiglio, status, ritorno;	/* per valore di ritorno figli */
	char ok='k',inpbuf;	      	/* carattere letto dai figli dalla pipe precedente e scritta su quella successiva */
	int nr,nw;              /* variabili per salvare valori di ritorno di read/write da/su pipe */
  char linea[255];
  int contaNum;

	/* controllo sul numero di parametri almeno 2 file */
	if (argc < 3){
		printf("Errore numero di parametri\n");
		exit(1);
	}

	Q = argc-1;
	printf("Numero di processi da creare %d\n", Q);

	/* allocazione pipe */
	if ((pipes=(pipe_t *)malloc(Q*sizeof(pipe_t))) == NULL){
		printf("Errore allocazione pipe\n");
		exit(2);
	}
	/* creazione pipe */
	for (q=0;q<Q;q++){
		if(pipe(pipes[q])<0){
			printf("Errore creazione pipe\n");
			exit(3);
		}
	}

	/* creazione figli */
	for (q=0;q<Q;q++){
		if ((pid=fork())<0)
		{
			printf("Errore creazione figlio\n");
			exit(4);
		}
    /* codice figlio */
		else if (pid == 0){

		/* chiusura pipes inutilizzate */
		for (j=0;j<Q;j++)
		{	/* si veda commento nella definizione dell'array pipes per comprendere le chiusure */
			if (j!=q)
				close (pipes[j][0]);
			if (j != (q+1)%Q)
				close (pipes[j][1]);
		}

		/* apertura file */
		if ((fd=open(argv[q+1],O_RDONLY))<0)
		{
			printf("Impossibile aprire il file %s\n", argv[q+1]);
			exit(-1);
		}

		/* inizializziamo l'indice dei caratteri letti per ogni singola linea */
    contaNum=0;
    j=0;
		/* con un ciclo leggiamo tutti i caratteri */
	  while(read(fd,&inpbuf,1)){
      if(inpbuf>=48 && inpbuf <=57)
        contaNum++;
      linea[j]=inpbuf;
      if(inpbuf=='\n'){
        linea[j]=0;
  			nr=read(pipes[q][0],&ok,sizeof(char));
  			/* per sicurezza controlliamo il risultato della lettura da pipe */
  			if (nr != sizeof(char))
  			{
  				printf("Figlio %d ha letto un numero di byte sbagliati %d\n", q, nr);
  				exit(-1);
  			}
  			printf("Figlio con indice %d e pid %d ha letto %d caratteri numerici nella linea %s\n",  q, getpid(), contaNum, linea);
  			nw=write(pipes[(q+1)%Q][1],&ok,sizeof(char));
  			if (nw != sizeof(char))
  			{
  				printf("Figlio %d ha letto un numero di byte sbagliati %d\n", q, nr);
  				exit(-1);
  			}
        ritorno=contaNum;
        contaNum=0;
        j=0;
      }
      else
        j++;

	  }


		/* ogni figlio deve tornare il numero di caratteri numerici dell'ultima linea */
		exit(ritorno);
		}
	}

	/* codice del padre */
	/* chiusura di tutte le pipe che non usa, a parte la prima perche' il padre deve dare il primo OK al primo figlio.
	N.B. Si lascia aperto sia il lato di scrittura che viene usato (e poi in effetti chiuso) che il lato di lettura
	(che non verra' usato ma serve perche' non venga inviato il segnale SIGPIPE all'ultimo figlio che terminerebbe in modo anomalo)  */
	for(q=1;q<Q;q++){ /* l'indice lo facciamo partire quindi da 1! (l'unica pipe che ci serve è quella del primo figlio)*/
		close (pipes[q][0]);
		close (pipes[q][1]);
	}
	/* ora si deve mandare l'OK al primo figlio (P0): nota che il valore della variabile ok (token) non ha importanza. controlliamo anche il risultato della scrittura */
	nw=write(pipes[0][1],&ok,sizeof(char));
	if (nw != sizeof(char)){
		printf("Padre ha scritto un numero di byte sbagliati %d\n", nw);
	        exit(5);
	}
	/* ora possiamo chiudere anche il lato di scrittura, ma ATTENZIONE NON QUELLO DI LETTURA!  (altrimenti va in SIGPIPE, perchè una pipe DEVE essere aperta sia in lettura che in scrittura)*/
	close(pipes[0][1]);


	/* Il padre aspetta i figli */
	for (q=0; q < Q; q++){
	  pidFiglio = wait(&status);
	  if (pidFiglio < 0)
	  {
	          printf("Errore in wait\n");
	          exit(6);
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
