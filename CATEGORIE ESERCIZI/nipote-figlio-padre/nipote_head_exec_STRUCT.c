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
	int c1; //pid nipote
	int c2; //numero della linea corrente
	char linea[250]; //contenuto linea corrente
}strut;

int mia_random(int n){
	int casuale;
	casuale = rand() % n;
	casuale++;
	return casuale;
}


int main(int argc, char **argv){
	int i=0, pidFiglio,ppid, status, ritorno, x,r;
	int n = (argc-1)/2; //numero dei processi figli
	char opzione[5];
	typedef int pipe_t[2];
	strut S;


	//CONTROLLO PARAMETRI
	if(argc < 3 || (argc-1)%2){
		printf("Errore nel numero di parametri\n");
		exit(1);
	}

	int *pid=malloc(sizeof(int)*n); //vettore di pid per salvarmi i pid dei figli

	//CREAZIONE N PIPE
	pipe_t *piped=malloc(sizeof(pipe_t)*(n));

	for(int c=0; c < n; c++){
		if (pipe (piped[c]) < 0){
			printf("Errore creazione pipe\n");
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
			if(k!=c)
				close(piped [k][1]);
		}

		//creazione pipe NIPOTE
		pipe_t pipedN;

		if (pipe (pipedN) < 0){
			printf("Errore creazione pipe\n");
			exit(-1);
		}		//CREAZIONE nipote
		if ((ppid = fork()) < 0){
			printf("Errore creazione nipote\n");
			exit(-1);
		}


		if(ppid==0){
			//CODICE NIPOTE
			srand(time(NULL));
			x=atoi(argv[(c*2)+2]);
			r=mia_random(x); //gli passo Xi+1
			sprintf(opzione, "-%d",r);
			close(piped[c][1]);
			close(1);
			dup(pipedN[1]); //ridirezione pipedN su stdout
			close(pipedN[0]);
			close(pipedN[1]);


			//Esecuzione head (che scrive su pipeN)
			execlp("head","head", opzione, argv[(c*2)+1],(char *) 0);
			perror("Errore comando head");
			exit(-1);
		}
		//FIGLIO

		S.c1=ppid;
		close(pipedN[1]);

		int j=0; //j conta la linea, i conta il carattere della linea corrente
		while(read(pipedN[0],&(S.linea[i]),1)){
			if(S.linea[i]=='\n'){
					j++;
					S.linea[i]=0;
					S.c2=j;
					write(piped[c][1],&S,sizeof(S));//scrive al padre la linea
					i=0;//azzera il contatore dei caratteri della linea
			}
			else
				i++;
		}
		exit(j);
	}
}


//PADRE
	printf("Padre %d sta per iniziare a leggere i messaggi della pipe \n", getpid());

	for(int k=0 ; k<n; k++){
		close (piped[k][1]);
	}

	bool finito=false;//variabile bool che controlla se tutti i figli sono terminati
	int ret; //variabile di ritorno della read per controllare che il figlio non sia terminato
	while(finito==false){
		finito=true;//suppongo che tutti i figli abbiano terminato
		for(i=0; i<n; i++) {
			//legge dalla pipe la struct
			ret=read(piped[i][0], &S, sizeof(S));
			//se almeno un figlio non è terminato, permetto un altro giro del while
			if(ret!=0){
				finito=false;
				printf("%d PID: %d LINEA: %d CONTENUTO: %s\n",i,S.c1,S.c2,S.linea);
			}
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
		exit (0);
}
