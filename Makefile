all : server client slave

CPPFLAGS = -g -I.

server : server.c
	gcc -g -I. server.c -o server -pthread

client : client.c
	gcc -g -I. client.c -o client -pthread

slave : slave.c
	gcc -g -I. slave.c -o slave -pthread

clean :
	rm server client slave
