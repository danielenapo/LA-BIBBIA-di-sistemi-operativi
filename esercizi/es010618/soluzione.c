/* Soluzione della parte C del compito della II prova in itinere del 1 Giugno 2018 */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

/* definiamo un tipo per un array di due interi */
typedef int pipe_t[2];

int main(int argc,char **argv)
{
int *pid, pidFiglio; 	/* array di pid per fork e pid per wait*/
int N;  		/* numero di file passati come parametri e numero di processi figli da creare */
int K;  		/* intero che rappresenta l'ultimo parametro passato */
int X;  		/* variabile che deve essere inserita da tastiera */
int L;  		/* lunghezza di ogni linea calcolata dai figli */
int status, ritorno;	/* variabili di stato per le wait e per il ritorno del figlio */
int trovata=0; 		/* indica se e' stata trovata la linea indicata */
int i,j;   		/* indici per cicli */
char linea[255]; 	/* array che serve per salvare la linea letta da ogni figlio, supposta non piu' lunga di 255 caratteri compreso il terminatore di linea (vedi commento riguardo a L nel testo) */
int fd;  		/* generico file descriptor */
pipe_t *piped;   	/* vettore dinamico per pipe figlio -> padre */
int nr;			/* per controllare valore di ritorno della read da pipe */

/* controlliamo numero di parametri */
/* devono esserci almeno tre parametri più argv[0] */
if (argc < 4)
{
   printf("Errore nel numero di parametri: %d. Ci vuole almeno due nomi di file e un numero che rappresenta il numero medio di linee dei file!\n", argc);
   exit(1);
}

/* calcoliamo il numero di files passati */
N=argc-2;

/* calcoliamo valore di K e lo controlliamo */
K=atoi(argv[argc-1]);
if (K < 0)
{
   printf("Errore valore di K\n");
   exit(2);
}

/* allocazione pid */
if ((pid=(int *)malloc(N*sizeof(int))) == NULL)
{
        printf("Errore allocazione pid\n");
        exit(3);
}

/* allocazione dinamica pipe figli-padre */
piped=(pipe_t *)malloc(sizeof(pipe_t)*N);
if (piped == NULL)
{
   printf("Allocazione fallita pipe figli-padre\n");
   exit(4);
}
/* creazione delle N pipe */
for (i=0;i<N;i++)
{
   if (pipe(piped[i])!=0)
   {
      printf("Errore pipe\n");
      exit(5);
   }
}
/* chiediamo all'utente di inserire un numero X da tastiera */
printf("Inserire un numero intero strettamente positivo X e minore o uguale a %d\n",K);
scanf("%d",&X);
/* controlliamo X */
if (X <= 0 || X > K)
{
   printf("Il valore X inserito non e' valido\n");
   exit(6);
}

/* ciclo per la generazione degli N figli */
for (i=0;i<N;i++)
{
   pid[i]=fork();
   if (pid[i]<0)  /* errore */
   {
      printf("Errore fork\n");
      exit(7);
   }
   if (pid[i]==0)  /* figlio i-esimo */
   {
      /* codice del figlio */
      /* stampa di controllo */
      printf("Figlio di indice %d e pid %d: selezionero' la linea %d-esima dal file %s\n",i,getpid(), X, argv[i+1]);
      /* chiudo le pipe che non servono */
      /* ogni figlio NON legge da nessuna pipe e scrive solo sulla propria piped[i] */
      for (j=0;j<N;j++)
      {
         close(piped[j][0]);
         if(j!=i)
         {
            close(piped[j][1]);
         }
      }

      /* apro il file di competenza, ossia quello di indice i+1 */
      fd=open(argv[i+1],O_RDONLY);
      if (fd < 0)
      {
         printf("Errore apertura file. Sono il figlio di indice %d\n",i);
         exit(-1); /* in caso di errore decidiamo di tornare -1 che sara' interpretato come 255, che NON e' un valore accettabile per il padre */
      }

      /* con un ciclo leggo la linea X-esima (se esiste) e ne calcolo la lunghezza */
      L=0; /* valore iniziale dell'indice */
      j=1; /* inizializzo il conteggio delle linee a 1 */
      while (read(fd,&(linea[L]),1) != 0)
      {
		if (linea[L] == '\n')
        	{
                	if (j == X) /* trovata la linea che dobbiamo selezionare e quindi la stampiamo dopo averla resa una stringa */
			{
				/* printf("STAMPA DI DEBUGGING: La linea numero %d del file %s e':\n%s", j, argv[i+1], linea); */
				L++; 	/* dobbiamo contare anche il terminatore di linea */
                        	trovata=1;
				ritorno=L;
				break; /* usciamo dal ciclo di lettura */
 			}			
                	else
                	{       L = 0; /* azzeriamo l'indice per la prossima linea */
                        	j++;    /* se troviamo un terminatore di linea incrementiamo il conteggio */
                	}
        	}	
        	else L++;
      }	
      if (!trovata)
      {
	/* se nel file non e' presente la linea X-esima dobbiamo mandare al padre comunque un valore di L e una 'linea': decidiamo di stabilire una frase che consenta al padre di capire che la linea non esiste */
	sprintf(linea, "Il file %s non contiene la linea %d-esima\n", argv[i+1], X);
	L = strlen(linea);
	ritorno=0;
      }
      /* comunichiamo L al processo padre */
      write(piped[i][1],&L,sizeof(L));
      /* comunichiamo la linea al processo padre */
      write(piped[i][1],linea, L);
  
      exit(ritorno);

      /* fine codice del figlio */
   }
}
/* codice del padre */
/* chiudo le pipe che non uso */
for (i=0;i<N;i++)
{
   close(piped[i][1]);
}
/* riceviamo dai figli le lunghezze calcolate, in ordine di indice e quindi utilizzando queste lunghezze riceviamo le linee inviate dai figli */
for (i=0;i<N;i++)
{
   nr=read(piped[i][0],&L,sizeof(int));
   if (nr == 0)
	printf("Il figlio con indice %d e pid %d ha avuto dei problemi\n",  i, pid[i]);
   else
   {
   read(piped[i][0],linea,L);
   /* il padre trasforma la linea ricevuta in una stringa in modo da non avere problemi: sovrascriviamo lo \n tanto nella printf seguente lo si aggiunge */
   linea[L-1]='\0';	
   printf("Il figlio con indice %d e pid %d mi ha comunicato questo: %s\n", i, pid[i], linea); 
   }
}

/* Attesa della terminazione dei figli */
for(i=0;i<N;i++)
{
   pidFiglio = wait(&status);
   if (pidFiglio < 0)
   {
      printf("Errore wait\n");
      exit(8);
   }
   if ((status & 0xFF) != 0)
                printf("Figlio con pid %d terminato in modo anomalo\n", pidFiglio);
   else
   {
                ritorno=(int)((status >> 8) & 0xFF);
                printf("Il figlio con pid=%d ha ritornato %d\n", pidFiglio, ritorno);
   }
}
exit(0);
}/* fine del main */

