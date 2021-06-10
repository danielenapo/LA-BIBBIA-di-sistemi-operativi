#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>

int main(int argc, char** argv){
    
    int n, fd;
    
    if (argc != 3){
        puts("Errore num argomenti");
        exit(1);
    }
    
    close(0);
    if((fd = open(argv[1],O_RDONLY))<0){
        puts("Errore apertura file");
        exit(2);
    }
    
    n=atoi(argv[2]);
    if(n<0){
        puts("Errore numero negativo");
        exit(3);
    }
    
    int contacar=0,nread,i=0;
    char c;
    bool trovata=false;
    char* stampa=malloc(0);
    while((nread = read(0, &c, 1))!=0){
        i++;
        stampa=realloc(stampa,i);
        stampa[i-1]=c;

        
        if(c=='\n'){
            if (n<=(i-1)){
                printf("%s", stampa);
                trovata=true;
            }
            contacar=0;
            stampa=malloc(0);
            i=0;
        }
    }
    
    if (trovata==false){
        puts("Errore nessuna linea trovata");
        exit(4);
    }
    
    exit(0);
}
