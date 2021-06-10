#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>

int main (int argc, char** argv){
    int pid;                                /* per fork */
    int pidFiglio, status, ritorno;         /* per wait padre */
    if (argc != 3){
    printf("Errore nel numero di parametri: %d\n", argc);
    exit(1);
}/* generiamo un processo figlio dato che stiamo simulando di essere il processo di shell! */
    pid = fork();
    if (pid < 0){       /* fork fallita */
        printf("Errore in fork\n");
        exit(2);
    }
    if (pid == 0){       /* figlio */
        printf("Esecuzione di grep da parte del figlio con pid %d\n", getpid());
        close(0);
        open(argv[2], O_RDONLY);
        close(1);
        open("/dev/null", O_WRONLY);
        close(2);
        open("/dev/null", O_WRONLY);
        execlp("grep", "grep", argv[1], (char *)0);
        printf("Errore in execlp\n"); 
        exit(-1); /* torniamo al padre un -1 che sara' interpretato come 255 e quindi identificato come errore */
    }
    pidFiglio = wait(&status);
    if (pidFiglio < 0){
        printf("Errore wait\n");
        exit(3);
    }
    if ((status & 0xFF) != 0)
    printf("Figlio con pid %d terminato in modo anomalo\n", pidFiglio);
    else{
        ritorno=(int)((status >> 8) & 0xFF);
        printf("Il figlio con pid=%d ha ritornato %d (se 255 problemi!)\n", pidFiglio, ritorno);
    }
    exit (0);
}
