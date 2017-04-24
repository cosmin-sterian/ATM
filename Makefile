build: client server

client: client.c utils.h
	gcc -g -Wall client.c -o client

server: server.c utils.h
	gcc -g -Wall server.c -o server

clean:
	rm -rf server client *.log
