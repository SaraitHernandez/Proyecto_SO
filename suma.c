#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>

int main(int argc, char *argv[]) {

	key_t key = ftok(".", 1234);
	int id_mem, *result;
	int a , b , c;
	void *pto_mem;

	//Memoria compartida

	if ((id_mem = shmget(key, 3 * sizeof(int), IPC_CREAT|0666)) < 0)
	{
		perror("shmget");
		exit(EXIT_FAILURE);
	} 
	if ((pto_mem = (void *) shmat(id_mem, NULL, 0)) == (int *) -1)
	{
		perror("shmmat");
		exit(EXIT_FAILURE);
	}

	result = (int *) pto_mem;
	
	a = atof ( argv [1]);
	b = atof ( argv [2]);
	c = a+b;
	result[0] = c;
	return 0;
}

