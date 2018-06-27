all : fibonacci server client slave collatz_t collatz_p fabrica

CPPFLAGS = -g -I.

server : server.c
	gcc -g -I. server.c -o server -pthread

client : client.c
	gcc -g -I. client.c -o client 

slave : slave.c
	gcc -g -I. slave.c -o slave -pthread

collatz_p : collatz_p.c
	gcc -g -I. collatz_p.c -o collatz_p 

collatz_t : collatz_t.c
	gcc -g -I. collatz_t.c -o collatz_t -pthread

fabrica : fabrica.c
	gcc -g -I. fabrica.c -o fabrica

fibonacci : fibonacci.c
	gcc -g -I. fibonacci.c -o fibonacci

clean :
	rm server client slave collatz_t collatz_p fabrica fibonacci
