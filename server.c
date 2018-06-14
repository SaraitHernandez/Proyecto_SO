#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h> 
#include <errno.h>
// Memomia compartida y cola de mensajes
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/ipc.h>
// sockets 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <netinet/in.h>

#include "server.h"


int main(int argc, char **argv)
{

  if(argc<2)
  {
    printf("%s [puerto]\n",argv[0]);
    return 1;
  }

  int puerto, process_pid[4], rc, on = 1;
  int listen_sd = -1, new_sd = -1, timeout;
  int current_size = 0, i, j;
  pid_t process;
  socklen_t longCliente;
  struct sockaddr_in servidor, cliente;
  char buffer[200];
  puerto = atoi(argv[1]);

  key_t key = ftok(".", 444);
  int id_mem;
  void *pto_mem;
  shmem_data *esclavos;

  /*
  * Memoria compartida 
  */
  if ((id_mem = shmget(key, sizeof(shmem_data), IPC_CREAT|0666)) < 0)
  {
    perror("shmget");
    exit(EXIT_FAILURE);
  } 
  if ((pto_mem = (void *) shmat(id_mem, NULL, 0)) == (int *) -1)
  {
    perror("shmmat");
    exit(EXIT_FAILURE);
  }

  esclavos = (shmem_data *) pto_mem;
  esclavos->nfds = 1;
  esclavos->nfds_dsp = 0;
  for (i = 0; i<200 ; i++)
    esclavos->dsp[i] = -1;

  /*
  * Asignacion -> socket
  */
  if ((listen_sd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    perror("socket() failed");
    exit(-1);
  }

  /*
  * Socket descriptor reutilizable
  */
  if ((rc = setsockopt(listen_sd, SOL_SOCKET,  SO_REUSEADDR,
                  (char *)&on, sizeof(on))) < 0)
  {
    perror("setsockopt() failed");
    close(listen_sd);
    exit(-1);
  }

  /*
  * Socket no bloqueante 
  */
  if((rc = ioctl(listen_sd, FIONBIO, (char *)&on)) < 0)
  {
    perror("ioctl() failed");
    close(listen_sd);
    exit(-1);
  }

  /*
  * Asignacion -> socket a puerto
  */
  bzero((char *)&servidor, sizeof(servidor));
  servidor.sin_family = AF_INET;
  servidor.sin_port = htons(puerto);
  servidor.sin_addr.s_addr = INADDR_ANY; //Macro -> Propia Direccion


  if(bind(listen_sd, (struct sockaddr *)&servidor, 
          sizeof(servidor)) < 0)
  {
    perror("bind() failed");
    close(listen_sd);
    exit(-1);
  }

  /*
  * Eschuchar
  */
  if ((rc = listen(listen_sd, 30)) < 0)
  {
    perror("listen() failed");
    close(listen_sd);
    exit(-1);
  }

  /*
  * Procesos encargados de: escuchar esclavos, 
  *escuchar al cliente, proceso encargado de 
  *los hijos que terminan
  */
  /*
  process_pid[0] = getpid();

  for(i=1; i <=3; i++)
  {
    process = fork();
    if (!process)
    {
      process_pid[i] = getpid();
      break;
    }else
      continue;  
  } 

  /*
  * Escuchar esclavos 
  */
  process = fork();
  if (!process) 
  {
    /*
    * Inicializar estructura pollfd 
    */
    memset(esclavos->fds, 0 , sizeof(esclavos->fds));
    esclavos->fds[0].fd = listen_sd; //servidor
    esclavos->fds[0].events = POLLIN; 
    timeout = (3 * 60 * 1000);

    /*
    *  Poll conexion de esclavos 
    */
    while(esclavos->nfds != 200)
    {
      if ((rc = poll(esclavos->fds, esclavos->nfds, timeout)) < 0)
      {
        perror("  poll() failed");
        continue;
      }
      if (rc == 0)
      {
        continue;
      }
      /*
      * Registrando esclavos
      */
      current_size = esclavos->nfds;
      for (i = 0; i < current_size; i++)
      {
        if(esclavos->fds[i].revents == 0)
          continue;

        if(esclavos->fds[i].revents != POLLIN)
        {
          printf("  Error! revents = %d\n", esclavos->fds[i].revents);
          break;
        }

        if (esclavos->fds[i].fd == listen_sd)
        {
          do
          {
            if((new_sd = accept(listen_sd, NULL, NULL)) < 0)
            {
              if (errno != EWOULDBLOCK)
                perror("  accept() failed");
              break;
            }
            esclavos->fds[esclavos->nfds].fd = new_sd; //tener cuidado cuando se desconecta un esclavo
            esclavos->fds[esclavos->nfds].events = POLLIN;
            esclavos->nfds++;
            esclavos->nfds_dsp++;
            esclavos->dsp[i] = 0;
          }while (new_sd != -1);
        }
      }
    }
  }else 
  {
    while(1)
    {

      i = 2;
      if (esclavos->fds[1].fd == listen_sd)
      {  
        do
        {
          send(esclavos->fds[1].fd, "Cliente", 200, 0);
          rc = recv(esclavos->fds[1].fd, buffer, sizeof(buffer), 0);
          if (rc < 0)
          {
            if (errno != EWOULDBLOCK)
            {
              perror("  recv() failed");
            }
            break;
          }

          if (rc == 0)
          {
            printf("  Connection closed\n");
            break;
          }

          for(i = 2; i < esclavos->nfds; i++)
          { 
            rc = send(esclavos->fds[i].fd, buffer, 200, 0);
            if (rc < 0)
            {
              perror("  send() failed");
              break;
            }
          }
        } while(1);
      }     
      sleep(10);
    }  
  }
  return 0;  
}


