CC  = gcc
INC =
OPC = -g -c
LIB =
obj = $(patsubst %.c,%.o,$(wildcard *.c))

.c.o:
	$(CC) $(OPC) $(INC) $<

all:	$(obj) ej1.out 

ej1 : eje1.c
	cc -g -I. eje1.c -o ej1

ej1.out: ej1.o
	$(CC) -o $@ $^

clean:
	rm -fr *.o *.out



#all : server client

#CPPFLAGS = -g -I.

#server : server.c
#	cc -g -I. server.c -o server

#client : client.c
#	cc -g -I. client.c -o client

#clean :
#	rm server client
