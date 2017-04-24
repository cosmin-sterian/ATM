#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_CLIENTS	5
#define BUFLEN 256

void error(char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
     int sockfd, newsockfd, portno, clilen;
     char buffer[BUFLEN];
     struct sockaddr_in serv_addr, cli_addr;
     int n, i, j, skip = 0;

     fd_set read_fds;	//multimea de citire folosita in select()
     fd_set tmp_fds;	//multime folosita temporar 
     int fdmax;		//valoare maxima file descriptor din multimea read_fds

     if (argc < 2) {
         fprintf(stderr,"Usage : %s port\n", argv[0]);
         exit(1);
     }

     //golim multimea de descriptori de citire (read_fds) si multimea tmp_fds 
     FD_ZERO(&read_fds);
     FD_ZERO(&tmp_fds);
     
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
        error("ERROR opening socket");
     
     portno = atoi(argv[1]);

     memset((char *) &serv_addr, 0, sizeof(serv_addr));
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;	// foloseste adresa IP a masiniia
     serv_addr.sin_port = htons(portno);
     
     if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr)) < 0) 
              error("ERROR on binding");
     
     listen(sockfd, MAX_CLIENTS);

     //adaugam noul file descriptor (socketul pe care se asculta conexiuni) in multimea read_fds
     FD_SET(sockfd, &read_fds);
     FD_SET(0, &read_fds);
     fdmax = sockfd;

     // main loop
	while (1) {
		tmp_fds = read_fds;
		select(fdmax+1, &read_fds, 0, 0, 0);
		skip = 0;
		for(i=0; i<= fdmax; i++)
		{
			if(FD_ISSET(i, &read_fds)) {
				if(i == 0) {
					//tastatura
					//printf("abc\n");
					fgets(buffer, BUFLEN - 1, stdin);
					j = atoi(&buffer[0]);
					if(FD_ISSET(j, &read_fds)) {
						printf("Sending to socket %d, message '%s'\n", j, buffer+2);
						write(j, buffer, strlen(buffer)+1);
					}
					if(strstr(buffer, "quit")!=NULL)
					{
						skip = 1;
						break;
					} else {
						printf("Type 'quit' to stop the server\n");
					}
				} else if(i == sockfd) {
					socklen_t clen = sizeof(cli_addr);
					newsockfd = accept(sockfd, (struct sockaddr*) &cli_addr, &clen);
					FD_SET(newsockfd, &tmp_fds);
					if(newsockfd > fdmax) {
						fdmax = newsockfd;
					}
					printf("New connection on socket: %d\n", newsockfd);
				} else {
					//mesaj
					if(read(i, buffer, BUFLEN) == 0) {
						FD_CLR(i, &tmp_fds);
						printf("Socket %d dropped.\n", i);
					} else {
						j = atoi(&buffer[0]);
						printf("Socket %d sent to %d message '%s'\n", i, j, buffer+2);
						write(j, buffer, strlen(buffer)+1);
					}
				}
			}
		}
		read_fds = tmp_fds;
		if(skip == 1)
			break;
     }


     close(sockfd);
   
     return 0; 
}


