#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
int main(int argc, char **argv){
   char buffer [BUFSIZ];
   int nread, fd = 0;
   
   for (int i=1; i<argc; i++){
           
       
       if ((fd = open(argv[i], O_RDONLY)) < 0){ 
           puts("Errore in apertura file");
           exit(2); }/* se non abbiamo un parametro, allora fd rimane uguale a 0 */
           
       while ((nread = read (fd, buffer, BUFSIZ)) > 0 )/* lettura dal file o dallo standard input fino a che ci sono caratteri */
           write(1, buffer, nread);/* scrittura sullo standard output dei caratteri letti */
           
        close(fd);
    }       

    return 0;
} 
