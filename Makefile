all : server client

CPPFLAGS = -g -I.

server : server.c
	cc -g -I. server.c -o server

client : client.c
	cc -g -I. client.c -o client

clean :
	rm server client
