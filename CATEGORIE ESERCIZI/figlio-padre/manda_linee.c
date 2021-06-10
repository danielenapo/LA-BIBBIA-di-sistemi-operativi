#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>



int main(int argc, char **argv){
	int  pidFiglio, status,ret,fd,k,x,l=0,i=0;
	char ch;
	char linea[255];
	bool trovato=false;
	typedef int pipe_t[2];


	//CONTROLLO PARAMETRI
	if(argc < 4){
		printf("Errore nel numero di parametri\n");
		exit(1);
	}
	int n = argc-2; //numero dei processi figli


	k=atoi(argv[argc-1]);
	if(k<0){
		puts("Errore numero negativo");
		exit(2);
	}

	//array pidFiglio
	int *pid=malloc(sizeof(int)*n);
	if(pid==NULL){
		puts("Errore allocazione");
		exit(3);
	}
	//CREAZIONE N PIPE
	pipe_t *piped=malloc(sizeof(pipe_t)*(n));
	if(piped==NULL){
		puts("Errore allocazione");
		exit(4);
	}

	//chiedo all'utente di inserire una variabile x minore di k e >0
	printf("inserire un numero <=k e >0:\n");
	scanf("%d",&x);
	//controllo sul valore di x
	if(x>k || x<0){
		perror("x valore sbagliato");
		exit(5);
	}

	//CICLO FIGLI
	for(int c=0; c < n; c++){
		if (pipe (piped[c]) < 0){
			printf("Errore creazione pipe\n");
			exit(6);
		}


		if ((pid[c] = fork()) < 0){
			printf("Errore creazione figlio\n");
			exit(7);
		}


	//FIGLIO C-ESIMO
	if(pid[c] == 0){
		//SCHEMA DI COMUNICAZIONE: ogni figlio manda al padre la lunghezza l e la linea, per ogni linea del file

		//chiudo pipe inutilizzate. ogni figlio non legge da nessuna pipe, scrive solo su piped[i] (comunicando col padre)
		for (int j=0; j<n; j++){
				close (piped [j][0]);
			if(j!=c)
				close(piped [j][1]);
		}

		//apertura file associato
		if((fd=open(argv[c+1],O_RDONLY))<0){
			puts("Errore apertura file");
			exit(-1);
		}

	//CONTA LUNGHEZZA LINEA + SALVATAGGIO linea
	//l è la lunghezza della linea, i è il contatore di linee
	//leggo un carattere alla volta dal file, quando trovo un '/n', salvo l e faccio write di l e la LINEA
	while(read(fd,&ch, 1)){
		linea[l]=ch;
		//QUANDO ARRIVO ALLA FINE DELLA LINEA
		if(ch=='\n'){
			linea[l]=0; //sostituisco il '\n' con il terminatore di stringa 0
			//se questa era la linea che dovevo salvare, finisco il ciclo e la scrivo al padre
			if(i==x){
				trovato=true;
				break;
			}
			i++;
			l=0; //azzero la lunghezza per la prossima linea
		}
		else
			l++;
	}

	//se esiste la linea x-esima, scrivo l e la linea, altrimenti scrivo 0 invece di l
	if(trovato==true){
		l++; //incremento per includere anche il terminatore
		write(piped[c][1],&l,sizeof(int));
		write(piped[c][1],linea,l);
	}
	else{
		l=0;
		write(piped[c][1],&l,sizeof(int));
		write(piped[c][1],linea,l);
	}


		exit(l); //torno la lunghezza della linea
	}
}


//PADRE
	printf("Padre %d sta per iniziare a leggere i messaggi della pipe \n", getpid());

	//chiusura pipe inutilizzate
	for(int k=0 ; k<n; k++){
		close (piped[k][1]);
	}

	//legge linea per ogni figlio (fa esattamente n letture)
	for(i=0; i<n; i++){
		read(piped[i][0],&l,sizeof(int));
		read(piped[i][0],linea,l);
		if(l!=0)
			printf("PID: %d, LUNGHEZZA: %d, CONTENUTO: %s\n", pid[i],l,linea);
		else
			printf("PID: %d LINEA NON ESISTENTE\n",pid[i]);
	}


//ASPETTA TUTTI I FIGLI
	for(int c=0; c<n; c++){
        pidFiglio = wait(&status);
        if (pidFiglio < 0){
            printf("Errore wait\n");
            exit(8);
        }

        if ((status & 0xFF) != 0)
            printf("Figlio con pid %d terminato in modo anomalo\n", pidFiglio);
        else{
            ret=(int)((status >> 8) & 0xFF);
            printf("Il figlio con pid=%d ha ritornato %d (se 255 problemi!)\n", pidFiglio,ret);
        }
    }
		exit (0);
}
