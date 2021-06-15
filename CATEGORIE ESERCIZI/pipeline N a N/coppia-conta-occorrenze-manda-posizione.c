/* soluzione parte C esame del 10 Luglio 2019 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

typedef int pipe_t[2]; 		/* tipo di dato per contenere i file descriptors di una pipe */

int main(int argc, char **argv)
{
	int N; 			/* numero di file: i processi figli saranno il doppio! */
	int pid;		/* variabile per fork */
	pipe_t *pipe_f;		/* array di pipe per la comunicazione dai primi N figli agli ultimi N figli */
	pipe_t *pipe_fbis;	/* array di pipe per la comunicazione dagli ultimi N figli ai primi N figli */
	/* OSSERVAZIONE: IN ALTERNATIVA POTEVA ESSERE USATO UN SOLO ARRAY DI DIMENSIONE 2 * N: CHIARAMENTE IN QUESTO CASO GLI INDICI DA USARE SAREBBERO DIVERSI RISPETTO A QUESTA SOLUZIONE! */
	int fd;			/* variabile per open */
	char ch;		/* variabile per leggere dai figli */
	char Cz;		/* variabile per tenere traccia del carattere da cercare */
	int occ;		/* variabile per tenere traccia del numero di occorrenze trovate */
	long int pos;		/* posizione corrente del carattere trovato: inviamo il valore ricavato dalla lseek decrementato di 1 dato che dopo la lettura l'I/O pointer e' posizionato sul carattere seguente quello letto */
	long int posLetta;	/* posizione corrente del carattere trovato ricevuta */
	int status, pidFiglio, ritorno;	/* per wait */
	int i, j;		/* indici per cicli */
	int nr, nw;		/* per controlli read e write su/da pipe */

  /* Controllo sul numero di parametri */
  if (argc < 4)
  {
  	printf("Errore numero parametri %d\n", argc);
  	exit(1);
  }
  N = argc - 2; //num di file

  // Controlliamo se l'ultimo parametro e' un singolo carattere
  if (strlen(argv[argc-1]) != 1){
    printf("Errore ultimo parametro non singolo carattere %s\n", argv[argc-1]);
    exit(2);
  }

  Cz = argv[argc-1][0]; /* isoliamo il carattere che devono cercare i figli */
  printf("Carattere da cercare %c\n", Cz);

  /* allocazione memoria dinamica per pipe_f e pipe_fbis */
  pipe_f=malloc(N*sizeof(pipe_t));
  pipe_fbis=malloc(N*sizeof(pipe_t));
  if ((pipe_f == NULL) || (pipe_fbis == NULL)){
  	printf("Errore nelle malloc\n");
  	exit(3);
  }

  /* creazione delle pipe */
  for (i=0; i < N; i++){
  	if (pipe(pipe_f[i])!=0){
  		printf("Errore creazione delle pipe primi N figli e gli ultimi N\n");
  		exit(4);
  	}
  	if (pipe(pipe_fbis[i])!=0){
  		printf("Errore creazione delle pipe ultimi N figli e i primii N\n");
  		exit(5);
  	}
  }

  /* creazione dei processi figli: ne devono essere creati 2 * N */
  for (i=0; i < 2*N; i++){
   	if ((pid=fork()) < 0) {
  		printf("Errore nella fork con indice %d\n", i);
      exit(6);
    }

    //CODICE FIGLIO
  	if (pid == 0){
      //primi n figli
      if (i < N){
  			printf("Figlio di indice %d e pid %d associato al file %s\n",i,getpid(), argv[i+1]);
  			//chiudiamo le pipe inutilizzate
  			/* ogni figlio scrive solo su pipe_f[i] e legge solo da pipe_fbis[i] */
  			for (j=0;j<N;j++){
    			close(pipe_f[j][0]);
    			close(pipe_fbis[j][1]);
    			if (j!=i){
    				close(pipe_f[j][1]);
    				close(pipe_fbis[j][0]);
    			}
  			}

  			/* per i primi N processi, il file viene individuato come al solito */
  			fd=open(argv[i+1], O_RDONLY);
  			if (fd < 0){
  				printf("Impossibile aprire il file %s\n", argv[i+1]);
  				exit(0); /* in caso di errore torniamo 0 che non e' un valore accettabile (per quanto risulta dalla specifica della parte shell) */
  			}

  			/* inizializziamo occ */
  			occ=0;
  	 		while (read(fd, &ch, 1)){
  				if (ch == Cz) { /* se abbiamo trovato il carattere da cercare */
  					occ++; //incremento occ
  					/* calcoliamo la posizione del carattere */
  					/* il valore ricavato dalla lseek lo decrementiamo di 1 dato che dopo la lettura l'I/O pointer e' posizionato sul carattere seguente quello letto */
  					pos=lseek(fd, 0L, SEEK_CUR) - 1;
  					/* inviamo la posizione del carattere all'altro processo della coppia */
  					nw=write(pipe_f[i][1], &pos, sizeof(pos));
  					if (nw != sizeof(pos)){
        			printf("Impossibile scrivere sulla pipe per il processo di indice %d\n", i);
        			exit(0);
     	 			}
  					/* aspettiamo dall'altro processo della coppia la nuova posizione da cui si deve riprendere la ricerca */
  					nr=read(pipe_fbis[i][0], &posLetta, sizeof(posLetta));
  					if (nr != sizeof(posLetta))
  						break;    // se non mi viene inviato alcuna posizione vuole dire che l'altro processo della coppia NON ha trovato altre occorrenze e quindi si può terminare la lettura

  					/* spostiamo l'I/O pointer nella posizione seguente! */
  					lseek(fd, posLetta+1, SEEK_SET);
  				}

  			}
  			exit(occ); /* torniamo il numero di occorrenze trovate (supposto dal testo <= di 255) */
  	   }

      //ultimi N figli
      else
    	{
    		printf("SECONDA SERIE DI FIGLI-Figlio di indice %d e pid %d associato al file %s\n",i,getpid(), argv[2*N-i]);
        /* chiudiamo le pipe che non servono */
        /* ogni figlio scrive solo su pipe_fbis[i] e legge solo da pipe_f[i] */
        for (j=0;j<N;j++){
          close(pipe_f[j][1]);
          close(pipe_fbis[j][0]);
          if (j!= 2*N-i-1){	 /* ATTENZIONE ALL'INDICE CHE DEVE ESSERE USATO */
            close(pipe_f[j][0]);
            close(pipe_fbis[j][1]);
          }
        }

  /* per gli ultimi N processi, il file viene individuato come indicato nel testo! */
        fd=open(argv[2*N-i], O_RDONLY);
        if (fd < 0){
          printf("Impossibile aprire il file %s\n", argv[2*N-i]);
          exit(0); /* in caso di errore torniamo 0 che non e' un valore accettabile (per quanto risulta dalla specifica della parte shell) */
        }
        /* inizializziamo occ */
        occ=0;
       	/* per prima cosa dobbiamo aspettare la posizione dall'altro figlio */
    		nr=read(pipe_f[2*N-i-1][0], &posLetta, sizeof(posLetta));
        if (nr != sizeof(posLetta)){
        	printf("Impossibile leggere dalla pipe per il processo di indice %d (PRIMA LETTURA)\n", i);
          exit(0);
        }
        /* spostiamo l'I/O pointer nella posizione seguente! */
        lseek(fd, posLetta+1, SEEK_SET);
        while (read(fd, &ch, 1)){
    			if (ch == Cz) {  /* se abbiamo trovato il carattere da cercare */
            /* incrementiamo occ */
            occ++;
            /* calcoliamo la posizione del carattere */
            /* il valore ricavato dalla lseek lo decrementiamo di 1 dato che dopo la lettura l'I/O pointer e' posizionato sul carattere seguente quello letto */
            pos=lseek(fd, 0L, SEEK_CUR) - 1;
            /* inviamo la posizione del carattere all'altro processo della coppia */
            nw=write(pipe_fbis[2*N-i-1][1], &pos, sizeof(pos));
            if (nw != sizeof(pos)){
              printf("Impossibile scrivere sulla pipe per il processo di indice %d\n", i);
              exit(0);
            }
            /* aspettiamo dall'altro processo della coppia la nuova posizione da cui si deve riprendere la ricerca */
            nr=read(pipe_f[2*N-i-1][0], &posLetta, sizeof(posLetta));
            if (nr != sizeof(posLetta))
    						break;		/* se non mi viene inviato alcuna posizione vuole dire che l'altro processo della coppia NON ha trovato altre occorrenze e quindi si può terminare la lettura */

  	        /* spostiamo l'I/O pointer nella posizione seguente! */
            lseek(fd, posLetta+1, SEEK_SET);
          }
        }
        exit(occ); /* torniamo il numero di occorrenze trovate (supposto dal testo <= di 255) */
      }
    }
  }

    /*codice del padre*/
    /* chiudiamo tutte le pipe, dato che le usano solo i figli */
    for (i=0;i<N;i++){
       close(pipe_f[i][0]);
       close(pipe_f[i][1]);
       close(pipe_fbis[i][0]);
       close(pipe_fbis[i][1]);
    }

    /* Attesa della terminazione dei figli */
    for(i=0;i<2*N;i++){
     pidFiglio = wait(&status);
     if (pidFiglio < 0){
      printf("Errore wait\n");
      exit(7);
     }
     if ((status & 0xFF) != 0)
      printf("Figlio con pid %d terminato in modo anomalo\n", pidFiglio);
     else{
      ritorno=(int)((status >> 8) & 0xFF);
      printf("Il figlio con pid=%d ha ritornato %d\n", pidFiglio, ritorno);
     }
    }
    exit(0);
  }
