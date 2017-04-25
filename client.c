#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "utils.h"

#define BUFLEN 256

void error(char *msg)
{
    perror(msg);
    exit(0);
}

void printTCP(FILE *log, char *string) {
	fprintf(log, "ATM> %s\n", string);
}

void printUDP(FILE *log, char *string) {
	if(strstr(string, "Trimite parola secreta") == NULL) {
		sprintf(string, "%s\n", string);
	}
	fprintf(log, "UNLOCK> %s", string);
}

int main(int argc, char *argv[])
{

    int sockfd;
    struct sockaddr_in serv_addr;
	char buffer[BUFLEN];

	sprintf(buffer, "client-%d.log", getpid());
	FILE *log = fopen(buffer, "w");
	memset(buffer, 0, BUFLEN);

    if (argc < 3) {
       fprintf(stderr,"Usage %s server_address server_port\n", argv[0]);
       exit(0);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening TCP socket");

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[2]));
    inet_aton(argv[1], &serv_addr.sin_addr);


    if (connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR connecting TCP");

    //UDP
    int sockfd_udp = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd_udp < 0)
    	error("ERROR opening UDP socket");

    if(connect(sockfd_udp, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0)
    	error("ERROR connecting UDP");

    fd_set read_fds, tmp_fds; //Am nevoie de un temp pentru ca altfel nu mai functioneaza bine multiplexarea, nu am idee de ce

    FD_ZERO(&read_fds);
	FD_ZERO(&tmp_fds);
    FD_SET(0, &read_fds);
    FD_SET(sockfd, &read_fds);
    FD_SET(sockfd_udp, &read_fds);
    int maxfd = (sockfd > sockfd_udp) ? sockfd : sockfd_udp;

	int logged_in = 0, quit = 0;
	int last_card = -1;
	socklen_t serv_addr_len = sizeof(serv_addr);
	char aux[BUFLEN];

	while(1) {
		tmp_fds = read_fds;
		select(maxfd+1, &tmp_fds, NULL, NULL, NULL);
		if(FD_ISSET(0, &tmp_fds)) {
			//Citire de la tastatura
			memset(buffer, 0, BUFLEN);
			fgets(buffer, BUFLEN, stdin);

			fprintf(log, "%s", buffer);

			if(strstr(buffer, "quit") != NULL) {
				memset(buffer, 0, BUFLEN);
				sprintf(buffer, "I'm going to disconnect.");
				write(sockfd, buffer, strlen(buffer)+1);
				break;
			}

			if(logged_in == 0 && checkInput(buffer) > 1 && checkInput(buffer) < 6) {
				fprintf(log, "-1 : Clientul nu este autentificat\n\n");
				continue;
			}

			switch(checkInput(buffer)) {
				case 1:	//login
					if(logged_in == 0) {
						write(sockfd, buffer, strlen(buffer)+1);
						sscanf(strchr(buffer, ' ')+1, "%d", &last_card);

					} else {
						//TODO eroare -2 deja logat
						fprintf(log, "-2 : Sesiune deja deschisa\n\n");
					}
					break;
				case 2:	//logout
					write(sockfd, buffer, strlen(buffer)+1);
					logged_in = 0;
					break;
				case 3:	//listsold
					write(sockfd, buffer, strlen(buffer)+1);
					break;
				case 4:	//getmoney
					write(sockfd, buffer, strlen(buffer)+1);
					break;
				case 5: //putmoney
					write(sockfd, buffer, strlen(buffer)+1);
					break;
				case 6:	//unlock
					sendto(sockfd_udp, buffer, strlen(buffer)+1, 0, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
					break;
				case 7:	//quit
					write(sockfd, buffer, strlen(buffer)+1);
					quit = 1;
					break;
				default:
					//Trimit parola pe UDP
					memset(aux, 0, BUFLEN);
					sprintf(aux, "%d %s", last_card, buffer);
					sendto(sockfd_udp, aux, strlen(aux)+1, 0, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
					break;
			}

		} else if(FD_ISSET(sockfd_udp, &tmp_fds)) {
			//Primesc informatii pe UDP
			if(recvfrom(sockfd_udp, buffer, BUFLEN, 0, (struct sockaddr*) &serv_addr, &serv_addr_len) > 0) {
				printUDP(log, buffer);
			}

		} else {
			//Primesc informatii pe TCP

			if(recv(sockfd, buffer, BUFLEN, 0) == 0) {
				error("[Client] Server closed the connection");
				break;
			} else {
				if(strstr(buffer, "Server is shutting down.") != NULL) {
					break;
				}

				printTCP(log, buffer);

				if(strstr(buffer, "Welcome") != NULL) {
					logged_in = 1;
				}
			}

		}

		if(quit == 1) {
			break;
		}
	}

	close(sockfd);
	fclose(log);
    return 0;
}
