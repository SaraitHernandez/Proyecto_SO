#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include  <sys/ipc.h>
#include  <sys/shm.h>
#include <sys/time.h>
#include <time.h>

typedef struct {
	unsigned int id;
	int n;
}t_hilo;
void * manejador_hilos(void * pto) {
	t_hilo *aux = (t_hilo *)pto; 
	int n = aux->n;
	while(n!=1){
		if (n%2 == 0) {
			n = n/2;
		} else {
			n = 3*n + 1;
		}
		
	}
	printf("%s %d\n", "Se cumple para el numero", aux->n);
	pthread_exit (( void *)(&(n)));
}

int main() {

	int *values = (int*)malloc(sizeof(int));
	pthread_t *info_hilos = (pthread_t*)malloc(sizeof(pthread_t));
	int count = 0;
	
	printf("%s\n", "intruduzca tanto numero como desee, 0 para terminar de introducir.");
	while(1){
		int n;
		scanf("%d", &n);
		if (n==0) {
			break;
		} else {
			values[count] = n;
			values = (int*)realloc(values, (count+2)* sizeof(int));
			info_hilos = (pthread_t*)realloc(info_hilos, (count+2)* sizeof(pthread_t));
			count++;
		}
	}

	t_hilo parametros_hilos[count];
	for (int i = 0; i < count; ++i) {
		parametros_hilos[i].id = i;
		parametros_hilos[i].n = values[i];
		pthread_create(&info_hilos[i],NULL,&manejador_hilos,(void *)&parametros_hilos[i]);
	}

	for (int i = 0; i < count; i++) {
		t_hilo *retorno;
	    pthread_join(info_hilos[i], (void **)&retorno);
 	}


	return 0;
}
