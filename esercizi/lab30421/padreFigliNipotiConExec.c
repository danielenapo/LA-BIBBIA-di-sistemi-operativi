#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>
#define PERM 0644

int main (int argc, char** argv){
    int pid;                                /* per fork */
    int n, pidFiglio, pidNipote, status, exit_s, ritorno,fd;         

    if (argc < 4){
        printf("Errore nel numero di parametri: ne hai messi %d, devono essere almeno 3\n", argc-1);
        exit(1);
    }
    
    n=argc-1;
    //creo n figli
    for (int i=0; i < n; i++){
        if ((pid=fork())<0){
            printf("Errore fork del figlio %d\n",i);
            exit(3);
        }
        //nel figlio conto le occorrenze di Cx
        else if (pid==0){
            //printf("FIGLIO %d con pid %d\n",i, getpid());
            char *nomefile=malloc(sizeof(argv[i+1])+6);
            strcpy(nomefile,argv[i+1]);
            strcat(nomefile,".sort");
            if((fd=creat(nomefile,PERM))<0){    //CREO FILE .sort
                puts("Errore creazione file");
                exit(-1);
            }
            //CREO NIMPOTE
            if ((pidNipote=fork())<0){
                puts("errore fork nipote");
                exit(-1);
            }
            else if(pidNipote==0){  //IL NIPOTE ORDINA IL FILE SU .sort CON COMANDO sort DELLA SHELL
                close(0);
                open(argv[i+1], O_RDONLY);
                close(1);
                open(nomefile,O_WRONLY);
                execlp("sort", "sort", (char *)0);
                perror("Errore in execlp\n"); 
                exit(-1); /* torniamo al padre un -1 che sara' interpretato come 255 e quindi identificato come errore */
            }
            //WAIT DEL FIGLIO PER LEGGERE IL RISULTATO DEL NIPOTE
            if ((pidFiglio=wait(&status)) < 0){
                printf("Errore in wait\n");
                exit(-1);
            }
            if ((status & 0xFF) != 0)
                printf("Nipote terminato in modo involontario (cioe' anomalo)\n");
            else{/* selezione del byte "alto" */
                ritorno = status >> 8;
                ritorno &= 0xFF;
                exit(ritorno);
            }
        }
    }
    
    //il padre stampa il risultato dei figli
    
    for(int i=0; i<n; i++){
        if ((pidFiglio=wait(&status)) < 0){
            printf("Errore in wait\n");
            exit(4);
        }
        if ((status & 0xFF) != 0)
            printf("Figlio terminato in modo involontario (cioe' anomalo)\n");
        else{/* selezione del byte "alto" */
            exit_s = status >> 8;
            exit_s &= 0xFF;
            printf("Per il figlio %d, lo stato di EXIT e` %d\n", pidFiglio, exit_s);
        }
    }



}
