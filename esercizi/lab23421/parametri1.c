#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>

int main (int argc, char** argv){

	if (argc < 2){
		printf("Errore: numero argomenti sbagliato");
		exit(1);
	}

	for (int i=0; i<argc; i++)
		printf("%s",argv[i]);

	return 0;
}
