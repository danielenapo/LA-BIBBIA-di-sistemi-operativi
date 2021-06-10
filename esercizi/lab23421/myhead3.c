#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

int main(int argc, char** argv){
    
    int n, fd;
    
    if (argc > 3){
        puts("Errore num argomenti");
        exit(1);
    }   

    
    if(argc==1)
        n=10;
    else{
        if(argv[1][0]!='-'){
            puts("Errore: il parametro deve essere preceduto dal segno -");
            exit(3);
        }
        n=atoi(argv[1]); //trasformo la stringa in numero
        n=n*(-1); //moltiplico per -1 perchè il parametro ha il - davanti in ogni caso
        
        //se c'è anche il nome del file, faccio la ridirezione
        if (argc==3){   
            close(0);
            if ((fd = open(argv[2], O_RDONLY)) < 0){ 
               puts("Errore in apertura file");
               exit(2); }/* se non abbiamo un parametro, allora fd rimane uguale a 0 */
        }
    }

    int contaline=0,nread;
    char c;
    while((nread = read(0, &c, 1))!=0){
        if(c=='\n')
            contaline++;
        if (contaline<n)
            write(1,&c,1);
    }
    printf("\n");
    exit(0);

}
