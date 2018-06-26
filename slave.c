#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/shm.h>
#include <signal.h>
#include <sys/sem.h>
#define SEM_ID 0x10101011

int llamadaSemaforo(int semId, int semNum, int op) 
{
  struct sembuf sops;
  sops.sem_num = semNum;
  sops.sem_op = op;
  sops.sem_flg = 0;
  return (semop(semId, &sops, 1)); /*devuelve -1 si error */
}

void exit_signal(int);

void run_function(int );

int main(int argc, char **argv)
{

  if(argc<2)
  {
    printf("[host] [puerto]\n");
    return 1;
  }
  
  int puerto, conexion;
  char buffer[200];
  char *a, *cl, *fn, msg[200], *tp, error[50];
  int n;
  pid_t porcess;
  key_t key = ftok(".", 1234);
  int id_mem, *result;
  void *pto_mem;


  //Memoria compartida

  if ((id_mem = shmget(key,  3 * sizeof(int), IPC_CREAT|0666)) < 0)
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

  int sem;

  if ((sem  = semget(SEM_ID, 1, IPC_CREAT | 0644)) < 0)
  {
    perror("Error al abrir el semaforo\n");
    return(-1);
  }
  semctl(sem, 0, SETVAL, 1);
  
  /* 
  * Estructura -> Informacion Conexion
  */  
  struct sockaddr_in cliente;
  
  /* 
  * Estructura -> Informacion Host
  */  
  struct hostent *servidor; 
  
  servidor = gethostbyname(argv[1]);
  puerto=(atoi(argv[2]));
  
  if(servidor == NULL)
  { 
    printf("Error\n");
    return 1;
  }
  
  /* 
  * Asignacion -> socket
  */ 
  conexion = socket(AF_INET, SOCK_STREAM, 0); //Asignación del socket

  /* 
  * Inicializacion -> 0 en todas las variables
  */  
  bzero((char *)&cliente, sizeof((char *)&cliente));
  
  cliente.sin_family = AF_INET;
  cliente.sin_port = htons(puerto);
  bcopy((char *)servidor->h_addr, (char *)&cliente.sin_addr.s_addr, sizeof(servidor->h_length));
 
  /* 
  * Estableciendo Conexion
  */
  if(connect(conexion,(struct sockaddr *)&cliente, sizeof(cliente)) < 0)
  { 
    printf("Error\n");
    close(conexion);
    return 1;
  }

 
  a = inet_ntoa(cliente.sin_addr);
 
  system("clear");
  printf("Conexion con -> %s:%d\n", a,htons(cliente.sin_port));
  while(1)
  {
    n = recv(conexion, buffer, 200, 0);
    if(n < 0)
      printf("error recv\n");
    else
    {
      strcpy(error, buffer);
      tp =  strtok(buffer, " ");
      if (strcmp(tp, "0") == 0)
      {
        fn =  strtok(NULL, " ");
        cl =  strtok(NULL, " ");
        printf("fd_client[%s], función a correr: %s\n",cl, fn);
        porcess = fork();
        if(!porcess)
        {
          //verifica la función que desea correr, la corre, retorna el resuldo
          llamadaSemaforo(sem, 0, -1);

          run_function(atoi(fn));
          snprintf(msg, sizeof(msg), "%s %d ", cl, result[atoi(fn)]);
          n= send(conexion, msg, 200, 0);
          if(n < 0)
            printf("error send\n");

          llamadaSemaforo(sem, 0, 1);
        }else
          continue; //sigue escuchando por si le mandan a correr otra cosa
      }else 
        printf("%s\n",error);
    }

    
  }
  return 0;
}

void run_function(int fn)
{
  int child;
  pid_t p;
  char *programs[4];


  p = fork();
  if(!p)
  {
    switch(fn)
    {
      case 0:
        programs[0] = "./suma";
        programs[1] = "5";
        programs[2] = "8";
        programs[3] = NULL;
        if(execvp(programs[0], programs) < 0)
          perror("exec");
        exit(1);
      case 1:
        programs[0] = "./fabrica";
        programs[1] = "2";
        programs[2] = NULL;
        if(execvp(programs[0], programs) < 0)
          perror("exec");
        exit(1);
      case 2:
        if(execvp(programs[2], programs) < 0)
          perror("exec");
        exit(1);
    } 
  }else
  {
    wait(&child);
    if(child)
      printf("error execvp\n");
    else
      printf("Finalizó la ejecución de la función requerida\n");
  }

}

void exit_signal(int num)
{
  key_t key_mem = ftok(".", 1234);
  int  id_mem;
  
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