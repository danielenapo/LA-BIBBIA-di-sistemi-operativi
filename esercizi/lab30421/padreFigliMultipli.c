#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>

int main (int argc, char** argv){
    int pid;                                /* per fork */
    int n, pidFiglio, status, exit_s;         /* per wait padre */
    if (argc != 2){
        printf("Errore nel numero di parametri: ne hai messi %d, deve essere 1\n", argc);
        exit(1);
    }
    n=atoi(argv[1]);    
    if(n<=0 || n>=255){
        printf("Errore numero primo parametro");
        exit(2);
    }
    
    for (int i=0; i < n; i++){
        if ((pid=fork())<0){
            printf("Errore fork del figlio %d\n",i);
            exit(3);
        }
        else if (pid==0){
            printf("FIGLIO %d con pid %d\n",i, getpid());
            exit(i);
        }
                
    }
    
    for(int i=0; i<n; i++){
        if ((pidFiglio=wait(&status)) < 0){
            printf("Errore in wait\n");
            exit(2);
        }
        else{       /* problemi */
            printf("Il pid della wait non corrisponde al pid della fork!\n");
            exit(3);
        }
        if ((status & 0xFF) != 0)
            printf("Figlio terminato in modo involontario (cioe' anomalo)\n");
        else{/* selezione del byte "alto" */
            exit_s = status >> 8;
            exit_s &= 0xFF;
            printf("Per il figlio %d lo stato di EXIT e` %d\n", pid, exit_s);
        }        
    }

    

    
    exit (0);
}


