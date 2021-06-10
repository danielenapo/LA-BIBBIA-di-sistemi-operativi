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
	int i, pidFiglio, status, ritorno,pid, fd;
	int n, j, numLinee, ret, maxIndex;
	char inpbuf,primoCar,x, maxChar;
	bool finito=false;

	//CONTROLLO PARAMETRI
	if(argc < 3){
		printf("Errore nel numero di parametri\n");
		exit(1);
	}
	n= argc-1; //numero dei processi figli
	//int *pid=malloc(sizeof(int)*n); //vettore di pid per salvarmi i pid dei figli

	//allocazione N PIPE DATI
	pipe_t *piped=malloc(sizeof(pipe_t)*(n));
	if(piped==NULL){
		puts("Errore allocazione array di pipe dati");
		exit(2);
	}
	//allocazione pipe token
	pipe_t *pipex=malloc(sizeof(pipe_t)*n);
	if(pipex==NULL){
		puts("Errore allocazione array di pipe token");
		exit(3);
	}

	//CICLO FIGLI
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
		//fork figlio
		if ((pid=fork()) < 0){
			printf("Errore creazione figlio\n");
			exit(6);
		}


	//FIGLIO C-ESIMO
	if(pid == 0){
		//SCHEMA DI COMUNICAZIONE:
		//il figlio legge la riga, manda il primo carattere al padre, legge il token dal padre e stampa l'output solo se questo contiene il carattere 's'

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
		j=0;//contatore lunghezza linea. mi serve solo per il primo carattere
		numLinee=0; //contatore delle linee (valore da ritornare nella exit)
		while(read(fd, &inpbuf, 1)){
			//se è il primo carattere lo salvo in una variabile che manderò al padre  a fine riga
			if(j==0)
				primoCar=inpbuf;
			//se arrivo alla fine della linea scrivo al padre il primo carattere, leggo il token e in base al suo valore scrivo in output o meno
			if(inpbuf=='\n'){
				numLinee++;
				j=0;
				write(piped[c][1],&primoCar,1);
				read(pipex[c][0],&x, 1);
				if(x=='s')
					printf("INDICE: %d, PID: %d, CARATTERE: %c, NUMLINEA: %d\n", c, getpid(), primoCar, numLinee);//stampa l’indice d’ordine del processo, il suo pid, il carattere identificato come massimoe quindila linea corrente
			}
			else
				j++;
		}

		exit(numLinee);
	}
}



//PADRE
	//chiude le piped inutilizzate
	for(int k=0 ; k<n; k++){
		close (piped[k][1]);
		close(pipex[k][0]);
	}

	printf("Padre %d sta per iniziare a leggere i messaggi della pipe \n", getpid());

	//vettore dei processi figli morti
	int *fdead=malloc(sizeof(int)*n);
	if(fdead==NULL){
		printf("errore allocazione array figli morti\n");
		exit(7);
	}
	//inizializzo tutti a 0. (se 0 è vivo, 1 è morto)
	for(i=0;i<n;i++){
		fdead[i]=0;
	}


	while(finito==false){
		maxChar=0;
		for(i=0; i<n; i++) {
			finito=true;
			if(fdead[i]==0){
				//legge dal figlio il suo carattere
				ret=read(piped[i][0],&inpbuf,1);
				if(ret==1){
					finito=false;
					if(inpbuf>=maxChar){
						maxChar=inpbuf;
						maxIndex=i;
					}
				}
				else
					fdead[i]=1;
				}
			}
			//manda token di scrittiura al figlio i-esimo (se non è già stato terminato)
			for(i=0; i<n; i++) {
				if(i==maxIndex)
					x='s';
				else
					x='n';
				if(fdead[i]==0){
					ret=write(pipex[i][1],&x,1);
					if(ret!=1){
						printf("Errore scrittura token\n");
						exit(8);
				}
			}
		}
	}



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
            ritorno=(int)((status >> 8) & 0xFF);
            printf("Il figlio con pid=%d ha ritornato %d (se 255 problemi!)\n", pidFiglio,ritorno);
        }
    }
		exit (0);
}
