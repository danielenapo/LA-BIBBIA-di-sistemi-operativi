#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <time.h>

int mia_random(int n){
	int casuale;
	casuale = rand() % n;
	return casuale;
}

int main(int argc, char **argv){
	srand(time(NULL));
	int i, pid, pidFiglio, status, ritorno, fd;
	int n = argc-2;
	char inpbuf,inpbuf1,ch;
	char vett[255];
	typedef int pipe_t[2];

	int fdTemp=open("/tmp/creato", O_RDWR | O_CREAT | O_APPEND);

	if(argc < 6){
		printf("Errore nel numero di parametri\n");
		exit(1);
	}

	int h=atoi(argv[argc-1]);
	if(h>255)
	{
		puts("Errore h >255");
		exit(2);
	}

//CREAZIONE N PIPE
	pipe_t *piped=malloc(sizeof(pipe_t)*(n));
	pipe_t *piped1=malloc(sizeof(pipe_t)*(n));

	for(int c=0; c < n; c++){
		if (pipe (piped[c]) < 0){
			printf("Errore creazione\n");
			exit(3);
		}

		if (pipe (piped1[c]) < 0){
			printf("Errore creazione\n");
			exit(3);
		}


		if ((pid = fork()) < 0){
			printf("Errore creazione figlio\n");
			exit(4);
		}

//FIGLIO C-ESIMO
	if(pid == 0){
		for (int k=0; k<n; k++){
			close (piped [k][0]);
			close(piped1[k][1]);
			if(k!=c){
				close(piped [k][1]);
				close(piped1[k][0]);
			}
		}
		//APERTURA FILE
		if((fd=open(argv[c+1], O_RDONLY)) < 0){
			printf("Errore in apertura file %s\n", argv[c+1]);
							exit(-1);
		}
		//CODICE
		int lunghezzaLinea=0;
		while((read(fd, &ch, 1))!=0){
			vett[lunghezzaLinea]=ch;
			lunghezzaLinea++;
			if(ch=='\n'){
				write(piped[c][1],&lunghezzaLinea, sizeof(int));
				read(piped1[c][0],&inpbuf,sizeof(int));
				if(lunghezzaLinea>inpbuf)
					write(fdTemp, &vett[inpbuf], 1);
				lunghezzaLinea=0;
			}
		}
		exit(0);
	}
}


//PADRE
	for(int k=0 ; k<n; k++){
		close (piped[k][1]);
		close (piped1[k][0]);
	}

	printf("Padre %d sta per iniziare a leggere i messaggi della pipe \n", getpid());
	int rand;
	int indice,rand2;
	for(i=0; i<h; i++) {
		rand=mia_random(n);
		for(int j=0; j<n; j++){
			read (piped[j][0], &inpbuf, sizeof(int));
			if(j==rand){
				indice=inpbuf;
			}
		}
		rand2=mia_random(indice);
		//printf("%d\n",indice);
		//SCRIVE L'INDICE A TUTTI I FIGLI
		for(int k=0; k<n; k++){
			write(piped1[k][1], &rand2, sizeof(int));
		}
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
		close(fdTemp);
		exit (0);
}
