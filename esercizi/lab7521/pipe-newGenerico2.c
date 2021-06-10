#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>
#define MSGSIZE 512

int main (int argc, char** argv){
    int pid;
    typedef int pipe_t[2];
    char msg; 
    char inpbuf;
    int pidFiglio, status, ritorno;

    if (argc < 2){
       printf("Numero dei parametri errato %d: ci vuole minimo un parametro\n", argc);
       exit(1);
    }
    /* CREAZIONE N PIPE */
    int n=argc-1;//numero di file
    pipe_t *piped=malloc(sizeof(pipe_t)*(n));
    for(int c=0; c< n; c++){
        if (pipe (piped[c]) < 0 ){
           printf("Errore creazione pipe\n");
           exit (2);
        }
        if ((pid = fork()) < 0){
           printf("Errore creazione figlio\n");
           exit (3);
        }
        
        /* FIGLIO C-ESIMO */
        if (pid == 0){
            int fd;
            //CHIUSURA PIPE INUTILIZZATE
            for(int k=0; k<n; k++){
                close (piped [k][0]);
                if(k!=c)
                    close(piped[k][1]);
            }      
            if ((fd = open(argv[c+1], O_RDONLY)) < 0){
               printf("Errore in apertura file %s\n", argv[1]);
               exit(-1);  
            }
            
            //LEGGO UN CARATTERE ALLA VOLTA E SCRIVO IN PIPE SOLO SE SODDISFA CERTI PARAMETRI   
            while (read(fd, &msg, 1)){
	            if(c%2==0){ //file pari: passo lettere
	                if(msg>=65 && msg<=122)
	                    write(piped[c][1], &msg, 1);
	            }
	            else{   //file dispari: passo numeri
	                if(msg>=48 && msg<=57)
	                    write(piped[c][1], &msg, 1);
	            }
            }
            //printf("Figlio %d scritto %d messaggi sulla pipe\n", getpid(), j);
            exit(0);
        }
    }
    
    /* PADRE */
    for(int k=0; k<n; k++)//chiude tutte le pipe di scrittura
        close (piped [k][1]); 
    printf("Padre %d sta per iniziare a leggere i messaggi dalla pipe\n", getpid());

    int i=0;
    while(read (piped[i][0], &inpbuf, 1)){
	    i++;
        if(i==n){
            i=0;
        }
	    printf ("%c", inpbuf);
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
