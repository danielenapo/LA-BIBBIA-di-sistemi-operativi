#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

int main(int argc, char** argv){
    
    if (argc > 2){
        puts("Errore num argomenti");
        exit(1);
    }
    
    int n;
    
    if(argc==1)
        n=10;
    else{
        if(argv[1][0]!='-'){
            puts("Errore: il parametro deve essere preceduto dal segno -");
            exit(2);
        }
        
        n=atoi(argv[1]); //trasformo la stringa in numero
        n=n*(-1); //moltiplico per -1 perchè il parametro ha il - davanti in ogni caso
    }
    char buff[BUFSIZ]; //creo una matrice di linee di stringhe
    char letto[BUFSIZ];
    int i=0; //scorre le linee
    int nread;
    while ((nread = read(0, buff, BUFSIZ)) > 0){ 
        if(i==0)
            strcpy(letto,buff);
        else if(i<n)
            strcat(letto,buff);
        i++;
    }  

    printf("%s",letto);
    
}
