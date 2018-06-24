all : server client

CPPFLAGS = -g -I.

server : server.c
	gcc -g -I. server.c -o server -pthread

client : client.c
	gcc -g -I. client.c -o client -pthread

clean :
	rm server client
