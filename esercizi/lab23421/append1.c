#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#define PERM 0644

int appendfile(char *f1)
{
    int outfile, nread;
    char buffer [BUFSIZ];
    
    /*APERTURA FILE*/
    if((outfile=open(f1, O_APPEND | O_WRONLY ,PERM))<0){ //se non esiste il file lo creo
        if((outfile=creat(f1,PERM))<0)
            return 1;
    }

    /*SCRITTURA IN APPEND*/
    while ((nread = read(0, buffer, BUFSIZ)) > 0){   
        if (write (outfile, buffer, nread ) < nread){ 
            close(outfile); 
            return 2;
        }
    }
    close(outfile); 
    return 0;
}

int main (int argc, char ** argv){ 
    int integi;
    if (argc != 2){ 
        printf ("ERRORE: ci vuole un argomento \n"); 
        exit (3); 
    }

    integi = appendfile(argv[1]);
    exit(integi);
}
