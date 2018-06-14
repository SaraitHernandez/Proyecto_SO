#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <errno.h>


int main(int argc, char **argv)
{


  if(argc<2)
  {
    printf("%s [puerto]\n",argv[0]);
    return 1;
  }

  int puerto, process_pid[4], rc, on = 1;
  int listen_sd = -1, new_sd = -1, timeout;
  pid_t process;
  socklen_t longCliente;
  struct sockaddr_in servidor, cliente;
  struct pollfd fds[5];
  int nfds = 1, current_size = 0, i, j;
  char buffer[200];
  puerto = atoi(argv[1]);
  
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
  if ((rc = listen(listen_sd, 5)) < 0)
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
 /* process_pid[0] = getpid();

  for(i=1; i <=3; i++)
  {
    process = fork();
    if (!process)
    {
      process_pid[i] = getpid();
      break;
    }else
      continue;  
  } */

  /*
  * Escuchar esclavos 
  */
  
  //if (getpid() == process_pid[1]) 
  //{
    /*
    * Inicializar estructura pollfd 
    */
    memset(fds, 0 , sizeof(fds));
    fds[0].fd = listen_sd;
    fds[0].events = POLLIN; 
    timeout = (3 * 60 * 1000);

    /*
    *  Registrar esclavos 
    */
    do
    {
      printf("Waiting on poll()...\n");
      if ((rc = poll(fds, nfds, timeout)) < 0)
      {
        perror("  poll() failed");
        break;
      }
      if (rc == 0)
      {
        printf("  poll() timed out.  End program.\n");
        break;
      }

    current_size = nfds;
    
    for (i = 0; i < current_size; i++)
    {
      if(fds[i].revents == 0)
        continue;

      if(fds[i].revents != POLLIN)
      {
        printf("  Error! revents = %d\n", fds[i].revents);
        break;
      }

      if (fds[i].fd == listen_sd)
      {

        printf("  Listening socket is readable\n");
        do
        {
          if((new_sd = accept(listen_sd, NULL, NULL)) < 0)
          {
            if (errno != EWOULDBLOCK)
            {
              perror("  accept() failed");
            }
            break;
          }

          printf("  New incoming connection - %d\n", new_sd);
          fds[nfds].fd = new_sd;
          fds[nfds].events = POLLIN;
          nfds++;

        }while (new_sd != -1);
      }
    }
  }while(nfds != 5);

    for (i = 0; i < nfds; i++)
    {
      if(fds[i].fd >= 0){
        printf("file descriptor[%d]: %d\n", i, fds[i].fd);
        close(fds[i].fd);
      }
    }

 // }

  return 0;  
}


