/* Soluzione della parte C del compito del 17 Febbraio 2021 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>

typedef int pipe_t[2];

int main (int argc, char **argv)
{
int Q; 			/* numero di file/processi */
/* ATTENZIONE NOME Q imposto dal testo! */
int pid;		/* pid per fork */
pipe_t *pipes;		/* array di pipe usate a ring da primo figlio, a secondo figlio .... ultimo figlio e poi a primo figlio: ogni processo legge dalla pipe q e scrive sulla pipe (q+1)%Q */
int q,j; 		/* indici */
/* ATTENZIONE NOME q imposto dal testo! */
int fd; 		/* file descriptor */
int pidFiglio, status, ritorno;	/* per valore di ritorno figli */
/* ATTENZIONE NOME linea imposto dal testo! */
char ok='k',inpbuf;	      	/* carattere letto dai figli dalla pipe precedente e scritta su quella successiva */
int nr,nw;              /* variabili per salvare valori di ritorno di read/write da/su pipe */
int k=0;

/* controllo sul numero di parametri almeno 2 file */
if (argc < 3)
{
	printf("Errore numero di parametri\n");
	exit(1);
}

Q = argc-1;
printf("Numero di processi da creare %d\n", Q);

/* allocazione pipe */
if ((pipes=(pipe_t *)malloc(Q*sizeof(pipe_t))) == NULL)
{
	printf("Errore allocazione pipe\n");
	exit(2);
}
/* creazione pipe */
for (q=0;q<Q;q++){
	if(pipe(pipes[q])<0)
	{
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
	else if (pid == 0){
	/* codice figlio */
	//printf("Sono il figlio %d e sono associato al file %s\n", getpid(), argv[q+1]);
	/* nel caso di errore in un figlio decidiamo di ritornare il valore -1 che sara' interpretato dal padre come 255 (valore NON ammissibile) */

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
	j = 0;
	/* con un ciclo leggiamo tutti i caratteri */
	int lol;
  while(read(fd,&inpbuf,1)){
			lol=q+k*Q;
			if(j==lol){
				nr=read(pipes[q][0],&ok,sizeof(char));
				/* per sicurezza controlliamo il risultato della lettura da pipe */
				if (nr != sizeof(char))
				{
						printf("Figlio %d ha letto un numero di byte sbagliati %d\n", q, nr);
						exit(-1);
				}
				printf("Figlio %d di numero %d ha carattere %c in posizione %d\n", getpid(), q, inpbuf, lol);
				k++;
				nw=write(pipes[(q+1)%Q][1],&ok,sizeof(char));
				if (nw != sizeof(char))
				{
						printf("Figlio %d ha letto un numero di byte sbagliati %d\n", q, nr);
						exit(-1);
				}

			}
			j++;
  }


	/* ogni figlio deve tornare il numero di caratteri numerici dell'ultima linea */
	exit(q);
	}
} /* fine for */

/* codice del padre */
/* chiusura di tutte le pipe che non usa, a parte la prima perche' il padre deve dare il primo OK al primo figlio. N.B. Si lascia aperto sia il lato di scrittura che viene usato (e poi in effetti chiuso) che il lato di lettura (che non verra' usato ma serve perche' non venga inviato il segnale SIGPIPE all'ultimo figlio che terminerebbe in modo anomalo)  */
for(q=1;q<Q;q++) /* l'indice lo facciamo partire quindi da 1! */
{
	close (pipes[q][0]);
	close (pipes[q][1]);
}
/* ora si deve mandare l'OK al primo figlio (P0): nota che il valore della variabile ok non ha importanza */
nw=write(pipes[0][1],&ok,sizeof(char));
/* anche in questo caso controlliamo il risultato della scrittura */
if (nw != sizeof(char))
{
	printf("Padre ha scritto un numero di byte sbagliati %d\n", nw);
        exit(5);
}
/* ora possiamo chiudere anche il lato di scrittura, ma ATTENZIONE NON QUELLO DI LETTURA! */
close(pipes[0][1]);
/* OSSERVAZIONE SU NON CHIUSURA DI pipes[0][0]: se si vuole procedere con la chiusura di tale lato nel padre, bisognerebbe introdurre del codice ulteriore solo nel primo figlio che vada a fare la lettura dell'ultimo OK prima di terminare! */
/* Il padre aspetta i figli */
for (q=0; q < Q; q++)
{
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
