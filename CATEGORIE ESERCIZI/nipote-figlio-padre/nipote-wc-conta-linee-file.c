#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

int main(int argc, char **argv){
	int i, pid, pidd, pidFiglio, pidNipote, status, status1, ritorno, ritorno1, ciao;
	long int tot=0;
	int n = argc-1;
	char inpbuf;
	char msg[4];
	typedef int pipe_t[2];

	if(argc < 2){
		printf("Errore nel numero di parametri\n");
		exit(1);
	}

//CREAZIONE N PIPE
	pipe_t *piped=malloc(sizeof(pipe_t)*(n));
	for(int c=0; c < n; c++){
		if (pipe (piped[c]) < 0){
			printf("Errore creazione\n");
			exit(2);
		}

		if ((pid = fork()) < 0){
			printf("Errore creazione figlio\n");
			exit(3);
		}
	
//FIGLIO C-ESIMO
	if(pid == 0){
		int fd;
		for (int k=0; k<n; k++){
			close (piped [k][0]);
			if(k!=c)
				close(piped [k][1]);
		}
		//CODICE
		pipe_t piped1;
		if ((pipe (piped1)) < 0){
			printf("Errore creazione\n");
			exit(2);
		}

		if ((pidd = fork()) < 0){
			printf("Errore creazione nipote\n");
			exit(4);
		}

		//NIPOTE
		if(pidd == 0){
			close(piped[c][1]);
			close(piped1[0]);
			close(0);
			if((open(argv[c+1], O_RDONLY)) < 0){
				printf("Errore in apertura file %s\n", argv[c+1]);
                exit(-1);  
			}
			close(1);
			dup(piped1[1]);
			close(piped1[1]);
			execlp("wc", "wc", "-l", (char *)0);
			exit(-1);
		}

		close(piped1[1]);
		read(piped1[0], &msg, 4);
		ciao = atoi(msg);
		write(piped[c][1], &ciao, sizeof(int));
		pidNipote = wait(&status1);
		if(pidNipote < 0){
			printf("Errore wait nipote\n");
            exit(6);
		}
		if ((status1 & 0xFF) != 0)
            printf("Nipote con pid %d terminato in modo anomalo\n", pidNipote);
        else{
            ritorno1=(int)((status1 >> 8) & 0xFF);
            printf("Il nipote con pid=%d ha ritornato %d (se 255 problemi!)\n", pidNipote,ritorno1);
        }

		exit(0);
		}
	}


//PADRE
	for(int k=0 ; k<n; k++){
		close (piped[k][1]);
	}

	printf("Padre %d sta per iniziare a leggere i messaggi della pipe \n", getpid());
	
	i=0;
	for(i=0; i<n; i++) {
		read (piped[i][0], &inpbuf, sizeof(int));
		//SOMMA VALORI
		tot += inpbuf;
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
    printf("Il totale delle linee e: %ld\n", tot);
    exit (0);
}