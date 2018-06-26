#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/shm.h>
#include <signal.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <time.h>
#include "fabrica.h"

int llamadaSemaforo(int semId, int semNum, int op) 
{
  struct sembuf sops;
  sops.sem_num = semNum;
  sops.sem_op = op;
  sops.sem_flg = 0;
  return (semop(semId, &sops, 1)); /*devuelve -1 si error */
}

void exit_signal(int);

int main(int argc, char *argv[]){

	key_t key_r = ftok(".", 1234);
	key_t key = ftok(".", 666);
	key_t key_q = ftok(".", 420);
	int id_mem, id_queue,i, tipo_msj=0, *contador_envios;
	int id_mem_2, *result, fin, total=0;
	void *pto_mem, *pto_mem_2;
	data_queue paquete;
	pid_t ensambladora;

	fin = atof ( argv [1]);
	//Memoria compartida resultado

	if ((id_mem_2 = shmget(key_r, 3 * sizeof(int), IPC_CREAT|0666)) < 0)
	{
		perror("shmget");
		exit(EXIT_FAILURE);
	} 
	if ((pto_mem_2 = (void *) shmat(id_mem_2, NULL, 0)) == (int *) -1)
	{
		perror("shmmat");
		exit(EXIT_FAILURE);
	}

	result = (int *) pto_mem_2;

	//Memoria compartida

	if ((id_mem = shmget(key, sizeof(int), IPC_CREAT|0666)) < 0)
	{
		perror("shmget");
		exit(EXIT_FAILURE);
	} 
	if ((pto_mem = (void *) shmat(id_mem, NULL, 0)) == (int *) -1)
	{
		perror("shmmat");
		exit(EXIT_FAILURE);
	}

	contador_envios = (int *) pto_mem;
	
	//Cola de mensajes

	if ((id_queue = msgget(key_q, IPC_CREAT|0666)) == -1)
	{
		perror("msgget");
		exit(EXIT_FAILURE);
	}

	//semaforo

	int sem;
  	if ((sem  = semget(SEM_ID, 1, IPC_CREAT | 0644)) < 0)
  	{
    	perror("Error al abrir el semaforo\n");
    	return(-1);
  	}
  	semctl(sem, 0, SETVAL, 1);
  
	//Fabricas

  	*contador_envios = 0;

	for(i=0; i<3; i++)
	{
		ensambladora = fork();
		if(!ensambladora){
			break;
		}
	}

	srand(time(NULL) + getpid());

	//envio de paquetes
	printf("fin %d\n",fin );
	if(!ensambladora){
		while(*contador_envios != fin){	

			sleep(3+(rand()%13));
			tipo_msj = 1 + (rand()%21);
			paquete.tipo_pieza = tipo_msj;
			paquete.contenido.cantidad_pieza = tipo_msj * 2;
			paquete.contenido.planta = getpid();
			
			llamadaSemaforo(sem, 0, -1);

			if (msgsnd(id_queue, (void *)&paquete, sizeof(data_queue), 0) == -1)
			{ 
	  			perror("Envio fallido");
	  		} else{
	  			(*contador_envios)++;
	  		}

	  		llamadaSemaforo(sem, 0, 1);
		}
	}else
	{
		signal(2, exit_signal);
		system("clear");
		printf("\n_______________________ Fabrica Ensambladora _______________________\n\n");
		while(*contador_envios != fin)
		{
			if(msgrcv(id_queue, (void *)&paquete, sizeof(data_queue), (long) 0, 0) == -1) 
        		perror("errormsgrcv\n");
    		else{ 
    			printf("La planta: %d envió el paquete nro: %d\n", paquete.contenido.planta, *contador_envios);
    			printf("De tipo: %ld\n", paquete.tipo_pieza);
    			printf("Con la cantidad de piezas: %d\n", paquete.contenido.cantidad_pieza);
    			total += paquete.contenido.cantidad_pieza;
    			fflush(stdout);
    		}

		}
	}

	result[1] = total;
	return 0;
}

void exit_signal(int num)
{
 	key_t key_mem = ftok(".", 666);
	key_t key_queue = ftok(".", 420);
	int  id_mem, id_queue;
	
	//Eliminar sección de memoria compartida
	
	if((id_mem = shmget(key_mem, sizeof(int), 0666)) < 0) 
  	{
		perror("shmget");
		exit(EXIT_FAILURE);
 	}
	
	if (shmctl(id_mem, IPC_RMID, 0) < 0) 
	{
		perror("shmctl(IPC_RMID)");
		exit(EXIT_FAILURE);
	}	
	
	//Eliminar cola de mensajes

	if((id_queue = msgget(key_queue, 0)) != -1) 
		msgctl(key_queue, IPC_RMID, 0);

	//Eliminar el semaforo

	int sem;
  	if ((sem  = semget(SEM_ID, 1, 0644)) < 0) {
    	perror("Error al abrir el semaforo\n");
    	exit(EXIT_FAILURE);
  	}

  	if (semctl(sem, 0, IPC_RMID, 0) == -1) 
  	{
	    perror("Error al eliminar el semaforo");
   		exit(EXIT_FAILURE);
  	}

	system("clear");
	printf("Hasta luego!\n");
    exit(EXIT_SUCCESS); 
}