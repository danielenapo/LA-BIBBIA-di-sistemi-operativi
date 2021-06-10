#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>
#define MSGSIZE 512

int main (int argc, char** argv){
    int pid,i;
    int piped[2];
    char msg; 
    char inpbuf;
    int pidFiglio, status, ritorno;

    //CONTROLLO PARAMETRI
    if (argc < 3){
       printf("Numero dei parametri errato %d: ci vuole minimo due parametri\n", argc);
       exit(1);
    }
    int n=argc-2;//numero di caratteri
    for(i=0; i<n; i++){
        if(argv[i+2][1]!=0){
            printf("errore parametro non lettera");
            exit(2);
        }
    }
    
    /* CREAZIONE PIPE (unica) */
    if (pipe (piped) < 0 ){
       printf("Errore creazione pipe\n");
       exit (3);
    }
    if ((pid = fork()) < 0){
       printf("Errore creazione figlio\n");
       exit (4);
    }
    
    /* FIGLIO */
    if (pid == 0){
        int fd;
        //CHIUSURA PIPE SCRITTURA
        close (piped[0]);
    
        if ((fd = open(argv[1], O_RDONLY)) < 0){    //APERTURA FILE
           printf("Errore in apertura file %s\n", argv[1]);
           exit(-1);  
        }
        
        //LEGGO UN CARATTERE ALLA VOLTA E INCREMENTO i OGNI VOLTA CHE TROVO L'OCCORRENZA (ripeto per tutti i caratteri)
        for(int c=0; c<n; c++){
            i=0; 
            while (read(fd, &msg, 1)){
                if(msg==argv[c+2][0]){
                    i++;
                    //printf("trovato %c -> %c; %d\n", msg, argv[c+2][0], i);
                }
            }
            //ALLA FINE SCRIVO IN PIPE LA LETTERA E IL RISPETTIVO NUMERO DI OCCORRENZE
            write(piped[1], &argv[c+2][0], 1);   
            write(piped[1], &i, sizeof(i)); 
            //sposto il puntatore del file all'inizio
            lseek(fd, 0L, SEEK_SET);
        }
        exit(0);
    }
    

    /* PADRE */
    //chiude pipe di scrittura
    close (piped[1]); 
    printf("Padre %d sta per iniziare a leggere i messaggi dalla pipe\n", getpid());

    //LEGGO CARATTERE E NUMERO DI OCCORRENZE (n volte, per ogni carattere)
    i=0;
    int occorrenze;
    for(i=0; i<n; i++){
    	read (piped[0], &inpbuf, 1);
	    read (piped[0], &occorrenze, sizeof(int));
	    printf("lettera %c: %d occorrenze\n", inpbuf, occorrenze);
    }

    //ASPETTA FIGLIO
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

    exit (0);
}
