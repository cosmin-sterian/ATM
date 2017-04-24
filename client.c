#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#define BUFLEN 256

void error(char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
	char buffer[BUFLEN];
    struct sockaddr_in serv_addr; 

    if(argc != 3)
    {
        printf("\n Usage: %s <ip> <port>\n", argv[0]);
        return 1;
    }

    // deschidere socket
    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sfd < 0) {
    	error("ERROR opening socket");
    }

    // completare informatii despre adresa serverului
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[2]));
    inet_aton(argv[1], &serv_addr.sin_addr);

    // conectare la server
    if(connect(sfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR connecting");    

    fd_set read_fds;
    FD_ZERO(&read_fds);


    FD_SET(0, &read_fds);
    FD_SET(sfd, &read_fds);
    int skip = 0;

    printf("Socket: %d\n", sfd);
    
    while(1){
        //tmp_fds = read_fds;
        //printf("Before select\n");
        int t = select(sfd + 1, &read_fds, 0, 0, 0);
        //printf("Select: %d\n", t);
        skip = 0;

        if(FD_ISSET(0, &read_fds)) {
        	//Citesc de la tastatura
        	fgets(buffer, BUFLEN, stdin);
        	write(sfd, buffer, strlen(buffer) + 1);
        	printf("Input: %s\n", buffer);
        	printf("Sending message '%s' to server's socket %d\n", buffer+2, atoi(&buffer[0]));
        } else {
        	//Primesc mesaj
        	if(read(sfd, buffer, BUFLEN) != 0)
            {
                printf("Received '%s' from client %d\n", buffer+2, buffer[0]);
            } else {
                printf("Server closed the connection.\n");
                skip = 1;
                break;
            }
        }

        /*for(i = 0; i <= fdmax; i++);
        {
            if(FD_ISSET(i, &tmp_fds)) {
                if(i == 0) {
                    fgets(buffer, BUFLEN, stdin);
                    write(sfd, buffer, strlen(buffer) + 1);
                    printf("Sending message '%s' to server's socket %d\n", buffer+2, buffer[0]);
                } else {
                    if(read(sfd, buffer, BUFLEN) != 0)
                    {
                        printf("Received '%s'\n", buffer);
                    } else {
                        printf("Server closed the connection.\n");
                        skip = 1;
                        break;
                    }
                }
            }
        }*/
        if(skip == 1)
        {
            break;
        }
    }
    close(sfd);
    return 0;
}


