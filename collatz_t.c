#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

char buffer[200];

typedef struct {
	unsigned int id;
	int n;
}t_hilo;

void * manejador_hilos(void * pto) {
	t_hilo *aux = (t_hilo *)pto; 
	int n = aux->n;
	char num[10];
	while(n!=1){
		snprintf(num, sizeof(num), "%d, ", n);
		strcat(buffer, num);
		if (n%2 == 0)
			n = n/2;
		else
			n = 3*n + 1;
	}
	pthread_exit ((void *)(&(n)));
}

int main(int argc, char *argv[]) {

	system("clear");
	printf("\nConjetura de Collatz_H\n\n");

	pthread_t info_hilos;
	int n;
	n = atof ( argv [1]);

	t_hilo parametros_hilos;
	
	parametros_hilos.id = 1;
	parametros_hilos.n = n;
	pthread_create(&info_hilos,NULL,&manejador_hilos,(void *)&parametros_hilos);

	t_hilo *retorno;
	pthread_join(info_hilos, (void **)&retorno);
 
	//strcpy(result[1], buffer); 

	return 0;
}
