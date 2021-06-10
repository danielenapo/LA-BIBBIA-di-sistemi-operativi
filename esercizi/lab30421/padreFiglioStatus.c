#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>

int main (){
    srand(time(NULL));
    int  ppid, pid,  pidFiglio,  status,  exit_s, random; 
    /*  pid del padre, pid  per  fork,  pidFiglio  e  status  per  wait,  exit_s  per selezionare valore di uscita figlio */
    ppid=getpid();
    printf("pid processo padre: %d \n", ppid);
    
    if ((pid = fork()) < 0){
       printf("Errore in fork\n");       /* fork fallita */
       exit(1);
    }
    if (pid == 0){               /* figlio */
        printf("Esecuzione del figlio\n");
        printf("processo figlio: %d, processo padre: %d\n",getpid(), getppid());
        random=rand()%100;
        sleep(4);        /* si simula con un ritardo di 4 secondi che il figlio faccia qualcosa! */
        exit(random);
    }
    /* padre */
    printf("Generato figlio con PID = %d\n", pid);
    /* il padre aspetta il figlio in questo caso interessandosi del valore della exit del figlio */
    if ((pidFiglio=wait(&status)) < 0){
        printf("Errore in wait\n");
        exit(2);
    }
    if (pid == pidFiglio)
        printf("Terminato figlio con PID = %d\n", pidFiglio);
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
    
    exit(0);
}
