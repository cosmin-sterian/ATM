#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define BUFLEN 256

void error(char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int sockfd;
    struct sockaddr_in serv_addr;

    char buffer[BUFLEN];
    if (argc < 3) {
       fprintf(stderr,"Usage %s server_address server_port\n", argv[0]);
       exit(0);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[2]));
    inet_aton(argv[1], &serv_addr.sin_addr);


    if (connect(sockfd,(struct sockaddr*) &serv_addr,sizeof(serv_addr)) < 0)
        error("ERROR connecting");

    fd_set read_fds, tmp_fds; //Am nevoie de un temp pentru ca altfel nu mai functioneaza bine multiplexarea, nu am idee de ce

    FD_ZERO(&read_fds);
	FD_ZERO(&tmp_fds);
    FD_SET(0, &read_fds);
    FD_SET(sockfd, &read_fds);

    while(1) {

		tmp_fds = read_fds;

        select(sockfd + 1, &tmp_fds, NULL, NULL, NULL);
        if(FD_ISSET(0, &tmp_fds)){
			//Citire de la tastatura
            memset(buffer, 0 , BUFLEN);
            fgets(buffer, BUFLEN, stdin);
            if(send(sockfd, buffer, strlen(buffer)+1, 0) < 0)
            	error("ERROR writing to socket");

        } else {

            if(recv(sockfd, buffer, BUFLEN, 0) == 0) {
				error("[Client] Server closed the connection.");
				break;
			} else {
				printf("[%d]: %s\n", atoi(&buffer[0]), buffer+1);
			}
		}
    }

	close(sockfd);
    return 0;
}
