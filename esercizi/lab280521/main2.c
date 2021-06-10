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

typedef struct{
	int c1; //carattere associato
	long int c2; //occorrenze del carattere nel file
}strut;



void bubbleSort(strut v[], int dim){
	 int i;
	 strut temp;
	  bool ordinato = false;
		while (dim>1 && !ordinato) {
			  ordinato = true; /* hp: è ordinato */
				for (i=0; i<dim-1; i++)
				if (v[i].c2>v[i+1].c2) {
					temp=v[i];
					v[i]=v[i+1];
					v[i+1]=temp;

					ordinato = false;
				}
					dim--;
			}
	}

int main(int argc, char **argv){
	int  pidFiglio, status, ritorno, fd;
	char inpbuf;
	int n = 26; //numero dei processi figli
	typedef int pipe_t[2];
	strut* vetS;


	//CONTROLLO PARAMETRI
	if(argc != 2){
		printf("Errore nel numero di parametri\n");
		exit(1);
	}



	int *pid=malloc(sizeof(int)*n); //vettore di pid per salvarmi i pid dei figli

	//CREAZIONE N PIPE
	pipe_t *piped=malloc(sizeof(pipe_t)*(n));


	//FOR DEI FIGLI
	for(int c=0; c < n; c++){
		//creazione n pipe
			if (pipe (piped[c]) < 0){
				printf("Errore creazione pipe\n");
				exit(3);
			}


		//fork di n figli
		if ((pid[c] = fork()) < 0){
			printf("Errore creazione figlio\n");
			exit(4);
		}


	//FIGLIO C-ESIMO
	if(pid[c] == 0){
		//lseek(fd, 0L, SEEK_SET);
		//apertura file
		if((fd=open(argv[1],O_RDONLY))<0){
			printf("Errore apertura");
			exit(4);
		}
		//chiudo pipe inutilizzate

		for (int k=0; k<n; k++){
			if(k!=c)
			close (piped [k][0]);
			if(k!=c+1)
				close(piped [k][1]);
		}

		vetS=malloc(sizeof(strut)*(c+1));
		if(vetS==NULL){
			puts("Errore allocazione");
			exit(-1);
		}

		//inizializzo la struttura del figlio
		vetS[c].c1=97+c;
		vetS[c].c2=0;

		//lettura dal figlio precedente
		if(c!=0){

			read(piped[c][0],vetS,sizeof(strut)*c);
		}

		for(int i=0; i<c; i++){
			printf("%c %ld\n",vetS[i].c1, vetS[i].c2);
		}
		//conta occorrenze
		while(read(fd,&inpbuf,1)){
			if(vetS[c].c1==inpbuf){
				vetS[c].c2++;
			}
		}
		//comunica al figlio sucessivo
		write(piped[c+1][1],vetS,sizeof(strut)*(c+1));

		close(fd);
		exit(inpbuf);
	}
}


//PADRE
	printf("Padre %d sta per iniziare a leggere i messaggi della pipe \n", getpid());

	for(int k=0 ; k<n; k++){
			close (piped[k][1]);
			if(k!=(n-1))
				close (piped[k][0]);
	}

	vetS=malloc(sizeof(strut)*n);

	read(piped[n][0],vetS,sizeof(strut)*n);
	bubbleSort(vetS,n);

	for(int i=0; i<n; i++){
		printf("PID: %d, CARATTERE: %c, OCCORRENZE: %ld\n",pid[i], vetS[i].c1, vetS[i].c2);
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
            printf("Il figlio con pid=%d ha ritornato il carattere %c con codie ascii %d (se 255 problemi!)\n", pidFiglio, ritorno, ritorno);
        }
    }
		exit (0);
}
