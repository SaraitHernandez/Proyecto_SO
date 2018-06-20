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
int timeout = (3 * 60 * 1000);


int main(int argc, char **argv)
{

  if(argc<2)
  {
    printf("%s [puerto]\n",argv[0]);
    return 1;
  }
   int    len ;

  int    desc_ready, end_server = 0, compress_array = 0;
  int    close_conn;
  int puerto, process_pid[4], rc, on = 1;
  int listen_sd = -1, new_sd = -1;
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
    * Inicializar estructura pollfd 
    */
    memset(esclavos->fds, 0 , sizeof(esclavos->fds));
    esclavos->fds[0].fd = listen_sd; //servidor
    esclavos->fds[0].events = POLLIN; 
    

    /*
    *  Poll conexion de esclavos 
    */
    do
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
        }else
        {
          printf("  Descriptor %d is readable\n", esclavos->fds[i].fd);
          close_conn = 0;
          do
          {
            rc = recv(esclavos->fds[i].fd, buffer, sizeof(buffer), 0);
            printf(" Buffer:  %s\n", buffer);
            if (rc < 0)
            {
              if (errno != EWOULDBLOCK)
              {
                perror("  recv() failed");
                close_conn = 1;
              }
              break;
            }

            if (rc == 0)
            {
              printf("  Connection closed\n");
              close_conn = 1;
              break;
            }

            len = rc;
            printf("  %d bytes received\n", len);

            rc = send(esclavos->fds[i].fd, "hola", len, 0);
            if (rc < 0)
            {
              perror("  send() failed");
              close_conn = 1;
              break;
            }
          } while(1);
      
          if (close_conn)
          {
            close(esclavos->fds[i].fd);
            esclavos->fds[i].fd = -1;
            compress_array = 1;
          }
        }  
        
        if (compress_array)
        {
          compress_array = 0;
          for (i = 0; i < esclavos->nfds; i++)
          {
            if (esclavos->fds[i].fd == -1)
            {
              for(j = i; j < esclavos->nfds; j++)
              {
                esclavos->fds[j].fd = esclavos->fds[j+1].fd;
              }
              i--;
              esclavos->nfds--;
            }
          }
        }
      }
    }while(end_server == 0); /* End of serving running.    */

     
    for (i = 0; i < esclavos->nfds; i++)
    {
      if(esclavos->fds[i].fd >= 0)
        close(esclavos->fds[i].fd);
    }

  return 0;  
}


