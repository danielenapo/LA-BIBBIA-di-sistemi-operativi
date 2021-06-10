#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv){
	if(argc != 4){
		printf("Errore: i parametri non sono 3");
		exit(1);
	}

	int f=open(argv[1], O_RDONLY);
	if(f == -1){
		printf("Errore: il primo parametro non è un file");
		exit(1);
	}
	close(f);

	if(argv[2] <= 0){
		printf("Errore: il secondo parametro non è positivo");
		exit(2);
	}

	if(strlen(argv[3])!= 1){
		printf("Errore: il terzo parametro non è un carattere");
		exit (3);
	}

	printf("esiste il file di nome %s", argv[1]);
	printf("il numero è %s", argv[2]);
	printf("il carattere finale è %s", argv[3]);
}
