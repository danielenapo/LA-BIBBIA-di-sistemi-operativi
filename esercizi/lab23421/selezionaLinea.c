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
    
    int contaline=1,nread;
    char c;
    bool trovata=false;
    while((nread = read(0, &c, 1))!=0){
        if (contaline==n){
            trovata=true;
            write(1,&c,1);
        }
        if(c=='\n')
            contaline++;
    }
    
    if (trovata==false){
        puts("Errore linea non presente (file troppo corto)");
        exit(4);
    }
    
    exit(0);
}
