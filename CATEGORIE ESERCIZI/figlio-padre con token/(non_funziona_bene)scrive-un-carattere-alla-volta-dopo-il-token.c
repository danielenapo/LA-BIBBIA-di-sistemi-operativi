#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>

int main(int argc, char **argv){
	int i, pidFiglio, status, ritorno;
	int n = argc-2; //numero dei processi figli
	char inpbuf,ch,x='x';
	typedef int pipe_t[2];



	//CONTROLLO PARAMETRI
	if(argc < 4){
		printf("Errore nel numero di parametri\n");
		exit(1);
	}

	int *pid=malloc(sizeof(int)*n); //vettore di pid per salvarmi i pid dei figli

	//CREAZIONE N PIPE DATI
	pipe_t *piped=malloc(sizeof(pipe_t)*(n));
	//creazione pipe segnale
	pipe_t *pipex=malloc(sizeof(pipe_t)*n);

	//CICLO FIGLI
	for(int c=0; c < n; c++){
		//pipe dati
		if (pipe (piped[c]) < 0){
			printf("Errore creazione\n");
			exit(3);
		}
		//pipe segnale
		if (pipe (pipex[c]) < 0){
			printf("Errore creazione\n");
			exit(3);
		}

		if ((pid[c] = fork()) < 0){
			printf("Errore creazione figlio\n");
			exit(4);
		}


	//FIGLIO C-ESIMO
	if(pid[c] == 0){
		//chiudo pipe inutilizzate
		for (int k=0; k<n; k++){
			close (piped [k][0]);
			close(pipex[k][1]);
			if(k!=c){
				close(piped [k][1]);
				close(pipex[k][0]);
			}
		}
		//APERTURA FILE
		close(0);
		if((open(argv[c+1], O_RDONLY)) < 0){
			printf("Errore in apertura file %s\n", argv[c+1]);
							exit(-1);
		}
		//CODICE
		while(read(pipex[c][0],&x,1)){ //il testo chiedeva esplicitamente di leggere solo dopo aver rivecuto un token
			//se il token x vale 't' (terminato), esce dal ciclo e fa exit(0)
			if(x=='t')
				break;
			//legge dal file un carattere
			read(0, &inpbuf, 1);
			//lo scrive in pipe
			write(piped[c][1],&inpbuf,1);
		}
		exit(0);
	}
}



//PADRE
	for(int k=0 ; k<n; k++){
		close (piped[k][1]);
		close(pipex[k][0]);

	}

	int af;
	//APERTURA FILE
	if((af=open(argv[argc-1], O_RDONLY)) < 0){
		printf("Errore in apertura file %s\n", argv[argc-1]);
						exit(5);
	}


	printf("Padre %d sta per iniziare a leggere i messaggi della pipe \n", getpid());
	int *fkill=malloc(sizeof(int)*n);
	for(i=0;i<n;i++){
		fkill[i]=0;
	}

	while((read(af, &inpbuf, 1))!=0){
		for(i=0; i<n; i++) {
			//manda segnale al figlio i-esimo (solo se non è già stato terminato)
			if(fkill[i]==0){
				write(pipex[i][1],&x,1);
				//legge dalla pipe il carattere e lo confronta con il suo
				read(piped[i][0],&ch,1);
				printf("%d %c\n",pid[i],ch);
				if(ch!=inpbuf)
					fkill[i]=1;
			}
		}
	}


	//controllo e uccido i figli che ho segnato su fkill, altrimenti do il segnale di stop
	x='t';
	for(i=0; i<n; i++){
		if(fkill[i]==1)
			kill(pid[i],SIGKILL);
		else
			write(pipex[i][1],&x,1);
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
