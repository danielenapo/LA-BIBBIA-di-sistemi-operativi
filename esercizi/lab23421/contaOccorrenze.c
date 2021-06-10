#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv){
	if(argc != 3){
		printf("Errore: i parametri non sono 2");
		exit(1);
	}

	int F=open(argv[1], O_RDONLY);
	if(F == -1){
		printf("Errore: il primo parametro non è un file");
		exit(1);
	}

	if(strlen(argv[2])!= 1){
		printf("Errore: il terzo parametro non è un carattere");
		exit (2);
	}

	printf("esiste il file di nome %s\n", argv[1]);
	printf("carattere da cercare: %s\n", argv[2]);
	
	char buffer[BUFSIZ];
	int nread;
	long c=0;
	while((nread= read(F, buffer, 1))>0){
		if(argv[2][0] == buffer[0]){
			c++;
		}
	}
	
	printf("numero di occorrenze: %ld", c);
	close(F);
}
