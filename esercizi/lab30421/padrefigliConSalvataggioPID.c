#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>

int main (int argc, char** argv){
    srand(time(NULL));
    int pid;                                /* per fork */
    int n, pidFiglio, status, exit_s;         /* per wait padre */
    if (argc != 2){
        printf("Errore nel numero di parametri: ne hai messi %d, deve essere 1\n", argc);
        exit(1);
    }

    n=atoi(argv[1]);
    if(n<=0 || n>=155){
        printf("Errore numero primo parametro");
        exit(2);
    }
    int* vet=malloc(n*sizeof(int));

    for (int i=0; i < n; i++){
        if ((pid=fork())<0){
            printf("Errore fork del figlio %d\n",i);
            exit(3);
        }
        else if (pid==0){
            printf("FIGLIO %d con pid %d\n",i, getpid());
            exit((rand()%(100+i)));
        }
        else //NON si deve modificare delle variabili solo nei figli, perchè non vale per il padre
            vet[i]=pid;
    }

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
            
            for (int j=0; j<n; j++){
                if(vet[j]==pidFiglio){
                    printf("Per il figlio %d di indice %d, lo stato di EXIT e` %d\n", pidFiglio, j, exit_s);
                    break;
                }
            }
        }
    }
    
    for (int j=0; j<n; j++)
        printf("%d\n",vet[j]);

    exit (0);
}
