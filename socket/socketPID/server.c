/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

void error(char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
     int sockfd, newsockfd, clilen;
     int portno=2525;
     char buffer[256];
     struct sockaddr_in serv_addr, cli_addr;
     int n;
     if (argc >1 ) {
         fprintf(stderr,"too many arguments\n");
         exit(1);
     }
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
        error("ERROR opening socket");
     bzero((char *) &serv_addr, sizeof(serv_addr));
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);

	 if (bind(sockfd, (struct sockaddr *) &serv_addr,
			  sizeof(serv_addr)) < 0) 
			  error("ERROR on binding");
	 listen(sockfd,5);
	 
	 while(1){	 
		 //CREAZIONE FIGLIO
		 int pid;
		 if ((pid = fork()) < 0){
			printf("Errore creazione figlio\n");
			exit(9);
		 }
		 //CODICE FIGLIO 
		 if(pid==0){
				clilen = sizeof(cli_addr);
			 newsockfd = accept(sockfd, 
						 (struct sockaddr *) &cli_addr, 
						 &clilen);
			 if (newsockfd < 0) 
				  error("ERROR on accept");
			 n = write(newsockfd,"Hello",5);
			 if (n < 0) error("ERROR writing to socket");
			 return 0; 
		}
	}
}
