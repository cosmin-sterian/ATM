#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "utils.h"

#define MAX_CLIENTS	5
#define BUFLEN 256

void error(char *msg)
{
    perror(msg);
    exit(1);
}

clients* readData(FILE *in, int n) {
	int i;
	clients *client = malloc(n * sizeof(clients));
	for(i = 0; i < n; i++) {
		fscanf(in, "%s %s %d %d %s %lf", client[i].nume, client[i].prenume, &(client[i].nr_card), &(client[i].pin), client[i].parola, &(client[i].sold));
		client[i].logged_in = 0;
		client[i].attempts_left = 3;
	}
	return client;
}

int findClient(int n, clients* client, int nr_card) {
	int i;
	for(i = 0; i < n; i++) {
		if(client[i].nr_card == nr_card)
			break;
	}
	if(i < n)
		return i;
	return -1;	//eroare
}

int main(int argc, char *argv[])
{
     int sockfd, newsockfd, portno;
     char buffer[BUFLEN];
     struct sockaddr_in serv_addr, cli_addr;

     fd_set read_fds;	//multimea de citire folosita in select()
     fd_set tmp_fds;	//multime folosita temporar
     int fdmax;		//valoare maxima file descriptor din multimea read_fds

     if (argc != 3) {
         fprintf(stderr,"Usage : %s port users_data_file\n", argv[0]);
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
	 int i, n, current, skip = 0, j;
	 int nr_card, pin;

	 FILE *in = fopen(argv[2], "r");
	 fscanf(in, "%d", &n);
	 clients *client = readData(in, n);
	 fclose(in);

	 debugPrint(client, n);

     // main loop
	while (1) {
		tmp_fds = read_fds;
		select(fdmax+1, &read_fds, NULL, NULL, NULL);
		skip = 0;
		current = -1;
		memset(buffer, 0, BUFLEN);

		nr_card = 0;
		pin = 0;

		for(i = 0; i <= fdmax; i++) {
			if(FD_ISSET(i, &read_fds)) {
				if(i == 0) {
					//tastatura
					//TODO
					fgets(buffer, BUFLEN, stdin);
					if(strstr(buffer, "quit") != NULL) {
						printf("Server is shutting down.\n");
						skip = 1;
						memset(buffer, 0, BUFLEN);
						sprintf(buffer, "Server is shutting down.");
						for(j = 1; j <= fdmax; j++) {
							write(j, buffer, strlen(buffer)+1);
							FD_CLR(j, &tmp_fds);
						}
						break;
					} else {
						printf("Use 'quit' to shut down.\n");
					}

				} else if(i == sockfd) {
					//Conexiune noua
					socklen_t clen = sizeof(cli_addr);
					newsockfd = accept(sockfd, (struct sockaddr*) &cli_addr, &clen);
					FD_SET(newsockfd, &tmp_fds);
					if(newsockfd > fdmax) {
						fdmax = newsockfd;
					}

				} else {
					//Un client deja existent interactioneaza cu serverul
					if(read(i, buffer, BUFLEN) == 0) {
						//Clientul s-a deconectat
						FD_CLR(i, &tmp_fds);
					} else {

						//printf("[SERVER] %s\n", buffer);
						if(strstr(buffer, "I'm going to disconnect.") != NULL) {
							FD_CLR(i, &tmp_fds);
							continue;
						}

						switch(checkInput(buffer)) {
							case 1:
								//login
								sscanf(strstr(buffer, "login")+6, "%d %d", &nr_card, &pin);
								current = findClient(n, client, nr_card);
								if(current == -1) {
									//Alt card decat cele primite in users_data_file
									memset(buffer, 0, BUFLEN);
									sprintf(buffer, "ATM> -4 : Numar card inexistent\n");
									write(i, buffer, strlen(buffer)+1);
									continue;
								}

								if(client[current].logged_in == 1) {
									//Clientul este deja logat in alta parte(daca era de la aceeasi conexiune, s-ar fi tratat cazul local)
									memset(buffer, 0, BUFLEN);
									sprintf(buffer, "ATM> -2 : Sesiune deja deschisa\n");
									write(i, buffer, strlen(buffer)+1);
								} else {
									//Prima autentificare cu acest card
									if(client[current].attempts_left == 0) {
										//Card blocat
										memset(buffer, 0, BUFLEN);
										sprintf(buffer, "ATM> -5 : Card blocat\n");
										write(i, buffer, strlen(buffer)+1);

									} else {
										//Cardul nu este blocat

										if(client[current].pin != pin) {
											//Pin invalid
											(client[current].attempts_left)--;
											memset(buffer, 0, BUFLEN);
											sprintf(buffer, "ATM> -3 : Pin gresit\n");
											write(i, buffer, strlen(buffer)+1);

										} else {
											//Pin corect
											client[current].logged_in = 1;
											memset(buffer, 0, BUFLEN);
											sprintf(buffer, "ATM> Welcome %s %s\n", client[current].nume, client[current].prenume);
											write(i, buffer, strlen(buffer)+1);
										}
									}
								}
								break;
							case 2:
								//logout

							default:
								printf("Not yet implemented :(\n");
								break;
						}
					}
				}
			}
		}

		read_fds = tmp_fds;
		if(skip == 1)
			break;
     }


     close(sockfd);
	 free(client);

     return 0;
}