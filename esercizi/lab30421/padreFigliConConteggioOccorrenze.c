#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>

int main (int argc, char** argv){
    int pid;                                /* per fork */
    int n, pidFiglio, status, exit_s;         /* per wait padre */
    if (argc < 4){
        printf("Errore nel numero di parametri: ne hai messi %d, devono essere almeno 3\n", argc-1);
        exit(1);
    }
    
    if(argv[argc-1][1]!=0){
        puts("Errore: l'ultimo parametro non è un singolo carattere");
        exit(2);
    }
    char Cx=argv[argc-1][0];
    n=argc-2;
    
    int occorrenze, fd;
    char buf;
    //creo n figli
    for (int i=0; i < n; i++){
        if ((pid=fork())<0){
            printf("Errore fork del figlio %d\n",i);
            exit(3);
        }
        //nel figlio conto le occorrenze di Cx
        else if (pid==0){
            printf("FIGLIO %d con pid %d\n",i, getpid());
            if((fd=open(argv[i+1], O_RDONLY))<0){
                puts("Errore apertura file");
                exit(-1);
            }
            occorrenze=0;
            while(read(fd,&buf,1)>0){
                if(buf==Cx)
                    occorrenze++;
            }
            exit(occorrenze);
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
