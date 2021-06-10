#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#define PERM 0644



int main(int argc, char **argv){
	int  pid, pidFiglio, status,ret,fd;
  int l, nLinee, returnL; //contatore lunghezza linee e numero delle linee
  int Z,z; //numero di file e indice del figlio (associato al file z-esimo)
  int createdF;//file descriptor del file creato
	char ch; //variabile per leggere i singoli caratteri dei file associati ai figli
	char line[210], nLineeString[5]; //variabile su cui si salva la linea (si suppone avere max 210 caratteri), e variabile su cui si salva il risultato della wc
	typedef int pipe_t[2];


	//CONTROLLO PARAMETRI
	if(argc < 3){
		printf("Errore nel numero di parametri\n");
		exit(1);
	}
	Z = argc-1; //numero dei processi figli

  //CREAZIONE file
  if((createdF=open("/tmp/danielenapolitano",O_CREAT |O_WRONLY, PERM))<0){
    puts("Errore creazione file");
    exit(2);
  }

  //FIGLIO SPECIALE PER CONTEGGIO NUMERO DI LINEE (nLinee)
  //pipe per comunicare il numero di linee
  pipe_t pipedS;
  if (pipe (pipedS) < 0){
    printf("Errore creazione pipe speciale\n");
    exit(3);
  }
  //fork figlio speciale
  if ((pid = fork()) < 0){
    printf("Errore creazione figlio speciale\n");
    exit(4);
  }

  //CODICE FIGLIO SPECIALE
  if(pid==0){
    //ridirezione input
    close(0);
    if((open(argv[1],O_RDONLY))<0){ //non ha importanza quale file apro: hanno tutti la stessa lunghezza. argv[1] esiste sicuramente
      puts("Errore apertura file figlio speciale\n");
      exit(-1);
    }
    close(pipedS[0]);//chiudo pipe speciale in lettura
    //ridirezione output
    close(1);
    dup(pipedS[1]);
    close(pipedS[1]); //chiudo pipe speciale in scrittura, ora inutile
    //esecuzione wc
    execlp("wc","wc","-l", (char *) 0);
    perror("Errore wc figlio speciale");
    exit(-1);
  }

  //CODICE PADRE
  close(pipedS[1]);//chiusura pipe speciale di scrittura
  ret=read(pipedS[0], nLineeString, 5);  //leggo il risultato della wc del figlio
  if(ret<0){
    puts("Errore lettura pipe figlio speciale\n");
    exit(5);
  }
  nLinee=atoi(nLineeString); //trasformo in int
  close(pipedS[0]);//ora chiudo anche l'altra pipe, dato che non mi serve più


	//ALLOCAZIONE N PIPE
	pipe_t *piped=malloc(sizeof(pipe_t)*(Z));
	if(piped==NULL){
		puts("Errore allocazione");
		exit(6);
	}

	//CICLO FIGLI
	for(z=0; z < Z; z++){
    //creazione pipe
		if (pipe (piped[z]) < 0){
			printf("Errore creazione pipe\n");
			exit(7);
		}
    //forlk figlio
		if ((pid = fork()) < 0){
			printf("Errore creazione figlio\n");
			exit(8);
		}


	//FIGLIO C-ESIMO
	if(pid == 0){
		//SCHEMA DI COMUNICAZIONE: ogni figlio manda al padre la lunghezza l e la linea, per ogni linea del file

		//chiudo pipe inutilizzate. ogni figlio non legge da nessuna pipe, scrive solo su piped[i] (comunicando col padre)
		for (int j=0; j<Z; j++){
				close (piped [j][0]);
			if(j!=z)
				close(piped [j][1]);
		}

		//apertura file associato
		if((fd=open(argv[z+1],O_RDONLY))<0){
			puts("Errore apertura file");
			exit(-1);
		}

	//CONTA LUNGHEZZA LINEA + SALVATAGGIO linea
	//l è la lunghezza della linea, i è il contatore di linee
	//leggo un carattere alla volta dal file, quando trovo un '/n', salvo l e faccio write di l e la LINEA
  l=0;//inizializzo il contatore di caratteri della linea
	while(read(fd,&ch, 1)){
		line[l]=ch;
		//QUANDO ARRIVO ALLA FINE DELLA LINEA
		if(ch=='\n'){
      //SCRITTURA IN PIPE AL PADRE: scrivo prima la lunghezza e poi la linea
      l++;//incremento l per includere il terminatore
      write(piped[z][1],&l,sizeof(int));
      write(piped[z][1],line,l);
      returnL=l;
			l=0; //azzero la lunghezza per la prossima linea
		}
		else
			l++;
	}
		exit(returnL); //torno la lunghezza dell'ultima linea letta
	}
}


//PADRE
	printf("Padre %d sta per iniziare a leggere i messaggi della pipe \n", getpid());

	//chiusura pipe inutilizzate (oosia tutte quelle di scrittura)
	for(int k=0 ; k<Z; k++){
		close (piped[k][1]);
	}

  for(int j=0; j<nLinee; j++){ //ciclo che scorre le linee
    for(int i=0; i<Z; i++){ //ciclo che scorre i figli
      //legge linea per ogni figlio (fa esattamente n letture), eseguo il controllo che la lettura in pipe sia avvenuta correttamente
  		ret=read(piped[i][0],&l,sizeof(int));
      if(ret!=sizeof(int)){
        puts("Errore lettura lunghezza linea\n");
        exit(9);
      }
  		ret=read(piped[i][0],line,l);
      if(ret!=l){
        puts("Errore lettura linea\n");
        exit(10);
      }
      //SCRITTURA SUL FILE CREATO ALL'INIZIO (e relativo controllo di corretta scrittura)
      ret=write(createdF, line, l);
      if(ret!=l){
        puts("Errore scrittura su file\n");
        exit(11);
      }
    }
  }

//ASPETTA TUTTI I FIGLI
	for(int z=0; z<Z; z++){
        pidFiglio = wait(&status);
        if (pidFiglio < 0)
            exit(12);
        if ((status & 0xFF) != 0)
            printf("Figlio con pid %d terminato in modo anomalo\n", pidFiglio);
        else{
            ret=(int)((status >> 8) & 0xFF);
            printf("Il figlio con pid=%d ha ritornato %d (se 255 problemi!)\n", pidFiglio,ret);
        }
    }
		exit (0);
}
