#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <stdbool.h>


int main(int argc, char **argv){
	int pidFiglio,ppid, status, ritorno, fd;
	char inpbuf;
	typedef int pipe_t[2];
	long contaf=0,contan=0; //variabili per il conteggio delle sostituzioni, distinte per figlio e nipote


	//CONTROLLO PARAMETRI
	if(argc<3){
		printf("Errore nel numero di parametri\n");
		exit(1);
	}
	int n = argc-1; //numero dei processi figli

	int *pid=malloc(sizeof(int)*n); //vettore di pid per salvarmi i pid dei figli
	if(pid==NULL){
		puts("Errore allocazione");
		exit(2);
	}

	//ALLOCAZIONE N PIPE padre-figlio
	pipe_t *piped=malloc(sizeof(pipe_t)*(n));
	if(piped==NULL){
		puts("Errore allocazione");
		exit(2);
	}
	//ALLOCAZIONE N PIPE padre-nipote
	pipe_t *pipedN=malloc(sizeof(pipe_t)*(n));
	if(pipedN==NULL){
		puts("Errore allocazione");
		exit(3);
	}


	//FOR CHE SCORRE I FIGLI (e relativi nipoti)
	for(int c=0; c < n; c++){
		//creazione pipe figlio
		if (pipe (piped[c]) < 0){
			printf("Errore creazione pipe\n");
			exit(3);
		}
		//creazione pipe nipote
		if (pipe (pipedN[c]) < 0){
			printf("Errore creazione pipe\n");
			exit(3);
		}
		//fork figlio
		if ((pid[c] = fork()) < 0){
			printf("Errore creazione figlio\n");
			exit(4);
		}


	//CODICE FIGLIO C-ESIMO
	if(pid[c] == 0){

		//fork nipote
		if ((ppid = fork()) < 0){
			printf("Errore creazione nipote\n");
			exit(-1);
		}

		//apertura file associato
		if((fd=open(argv[c+1],O_RDWR))<0){
			perror("Errore apertura file");
			exit(-1);
		}

		//CODICE NIPOTE
		if(ppid==0){

			//chiude pipe inutilizzate
			for (int k=0; k<n; k++){
				close (pipedN [k][0]);
				close(piped[k][1]);
				close(piped[k][0]);
				if(k!=c)
					close(pipedN [k][1]);
			}

			//legge il file carattere per carattere e trasforma le lettere minuscole in maiuscole
			//incrementa il contatore ad ogni trasformazione
			while(read(fd,&inpbuf,1)){
				if(inpbuf>=97 && inpbuf <= 122){
					contan++; //incremento il contatore delle sostituzioni del nipote
					inpbuf-=32; //trasformo in maiuscolo
					lseek(fd,-1,SEEK_CUR);
					write(fd,&inpbuf,1); //sostituisco il carattere
				}
			}
			//al termine della lettura, scrivo al padre il numero di sostituzione e faccio exit(0)
			write(pipedN[c][1],&contan,sizeof(long));
			exit(0);
		}

		//chiudo pipe inutilizzate
		for (int k=0; k<n; k++){
			close (piped [k][0]);
			close(pipedN[k][0]);
			close(pipedN[k][1]);
			if(k!=c)
				close(piped [k][1]);
		}
		//ASPETTA NIPOTE
    ppid = wait(&status);
    if (ppid < 0){
        printf("Errore wait\n");
        exit(5);
    }

    if ((status & 0xFF) != 0)
        printf("Nipote con pid %d terminato in modo anomalo\n", ppid);
    else{
        ritorno=(int)((status >> 8) & 0xFF);
        printf("Il nipote con pid=%d ha ritornato %d (se 255 problemi!)\n", ppid,ritorno);
    }

		//legge il file carattere per carattere e trasforma i numerici in spazio
		//incrementa il contatore ad ogni trasformazione
		while(read(fd,&inpbuf,1)){
			if(inpbuf>=48 && inpbuf <= 57){
				contaf++; //incremento il contatore delle sostituzioni del figlio
				inpbuf=' '; //trasformo in spazio
				lseek(fd,-1,SEEK_CUR);
				write(fd,&inpbuf,1); //sostituisco il carattere
			}
		}
		//al termine della lettura, scrivo al padre il numero di sostituzione e faccio exit(0)
		write(piped[c][1],&contaf,sizeof(long));
		exit(0);
	}
}


//PADRE
	printf("Padre %d sta per iniziare a leggere i messaggi della pipe \n", getpid());

	for(int k=0 ; k<n; k++){
		close (piped[k][1]);
		close(pipedN[k][1]);
	}

	//per ogni file legge il numero di trasformazioni di figlio e nipote
	for(int i=0; i<n; i++){
		read(pipedN[i][0],&contan,sizeof(long));
		read(piped[i][0],&contaf,sizeof(long));

		//stampa valori
		printf("FILE: %s, TRASFORMAZIONI -> FIGLIO: %ld, NIPOTE: %ld\n", argv[i+1],contaf, contan);
	}


//ASPETTA TUTTI I FIGLI
	for(int c=0; c<n; c++){
        pidFiglio = wait(&status);
        if (pidFiglio < 0){
            printf("Errore wait\n");
            exit(5);
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
