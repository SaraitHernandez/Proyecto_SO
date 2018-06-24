#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <errno.h>
#include <pthread.h>
// Sockets
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <netinet/in.h>

#include "server.h"

#define NRO_THR 4

pthread_t threads[NRO_THR];
struct pollfd fds_slaves[20], fds_clients[10], fd_server_client;
int result, nfds_slaves = 0, nfds_clients = 0;
int listen_slave = -1, listen_client = -1;
int timeout = (3 * 60 * 1000);

data t_arg;
data_slave reg_slv;
data_client reg_cli; 

void * registering_clients() 
{
  int rc, new_sd = -1, i;
  int nfds = 1;
  while(1)
  {
    if (((rc = poll(&fd_server_client, nfds, timeout)) < 0))
    {
      perror("poll() failed");
      continue;
    }
    
    if (rc == 0) 
    {
      continue;
    }

    if (fd_server_client.fd == listen_client)
    {
      do
      {
        if((new_sd = accept(listen_client, NULL, NULL)) < 0)
        {
            if (errno != EWOULDBLOCK)
                perror("accept() failed");
            break;
        }
        fds_clients[nfds_clients].fd = new_sd; //tener cuidado cuando se desconecta un esclavo
        fds_clients[nfds_clients].events = POLLIN;
        nfds_clients++;
        printf("nfds_clients %d\n", nfds_clients);
      }while (new_sd != -1);
    }
  }
  
}

void * send_msg_client() 
{
  int current_size, fd, rc, len, close_conn, i, j;
  int end_server = 0, compress_array = 0;
  char buffer[200];

  do
  {
    while(1)
    {
      if(nfds_clients >= 1)
      {
    
        close_conn = 0;

        if ((rc = poll(fds_clients, nfds_clients, timeout)) < 0)
        {
          perror("poll() failed");
          continue;
        }
        
        if (rc == 0)
        {
          printf("timeout\n");
          continue;
        }

        current_size = nfds_clients;
        for (i = 0; i < current_size; i++)
        {
          
          if(fds_clients[i].revents == 0)
          {
              printf("Nothing to read.\n");
              continue;
          }

          if(fds_clients[i].revents != POLLIN)
          {
              printf("Error revents  padre= %d\n", fds_clients[fd].revents);
              close_conn = 1;
              break;
          }

          if (fds_clients[i].fd == listen_client)
            continue;
          else 
          {
            printf("    Descriptor %d is readable\n", fds_clients[fd].fd);

            rc = recv(fds_clients[i].fd, buffer, sizeof(buffer), 0);

            if (rc < 0)
            {
                if (errno != EWOULDBLOCK)
                {
                    perror("    recv() failed");
                    close_conn = 1;
                }
                break;
            }

            if (rc == 0)
            {
                printf("    Connection closed\n");
                close_conn = 1;
                break;
            }

            len = rc;
            printf("    %d bytes received\n", len);
            printf(" Buffer:    %s\n", buffer);

            rc = send(fds_clients[i].fd, "hola:)", 16, 0);
            if (rc < 0)
            {
              perror("    send() failed");
              close_conn = 1;
              break;
            }
          }
      
          if (close_conn)
          {
            close(fds_clients[i].fd);
            compress_array = 1;
          }
        }       
        if (compress_array)
        {
          for (i = 0; i < nfds_clients; i++)
          {
            if (fds_clients[i].fd == -1)
            {
              for(j = i; j < nfds_clients; j++)
              {
                  fds_clients[j].fd = fds_clients[j+1].fd;
              }
              i--;
              nfds_clients--;
              fds_clients[nfds_clients].fd = -1;
            }
          }
        }
      }else
        printf("nfds_clients_3 %d\n", nfds_clients), sleep(5);
    }
  }while(end_server == 0);
  
}

int main(int argc, char **argv)
{
    if(argc<3)
    {
        printf("%s [port for the slave] [port for the client]\n",argv[0]);
        return 1;
    }
    
    int port_slave, port_client, rc, on = 1, i;
    data_client *retorno;
    struct sockaddr_in client, slave;
    port_slave = atoi(argv[1]);
    port_client = atoi(argv[2]);

    /*
    * Asignacion -> socket
    */
    if ((listen_slave = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket() failed");
        exit(-1);
    }

    if ((listen_client = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket() failed");
        exit(-1);
    }

    /*
    * Socket descriptor reutilizable
    */
    if ((rc = setsockopt(listen_slave, SOL_SOCKET,  SO_REUSEADDR,
                                    (char *)&on, sizeof(on))) < 0)
    {
        perror("setsockopt() failed");
        close(listen_slave);
        exit(-1);
    }

    if ((rc = setsockopt(listen_client, SOL_SOCKET,  SO_REUSEADDR,
                                    (char *)&on, sizeof(on))) < 0)
    {
        perror("setsockopt() failed");
        close(listen_client);
        exit(-1);
    }

    /*
    * Socket no bloqueante
    */
    if((rc = ioctl(listen_slave, FIONBIO, (char *)&on)) < 0)
    {
        perror("ioctl() failed");
        close(listen_slave);
        exit(-1);
    }

    if((rc = ioctl(listen_client, FIONBIO, (char *)&on)) < 0)
    {
        perror("ioctl() failed");
        close(listen_client);
        exit(-1);
    }
    /*
    * Asignacion -> socket a puerto 
    */
    bzero((char *)&slave, sizeof(slave));
    slave.sin_family = AF_INET;
    slave.sin_port = htons(port_slave);
    slave.sin_addr.s_addr = INADDR_ANY; //Macro -> Propia Direccion

    bzero((char *)&client, sizeof(client));
    client.sin_family = AF_INET;
    client.sin_port = htons(port_client);
    client.sin_addr.s_addr = INADDR_ANY;


    if(bind(listen_slave, (struct sockaddr *)&slave,
                    sizeof(slave)) < 0)
    {
        perror("bind() failed");
        close(listen_slave);
        exit(-1);
    }

    if(bind(listen_client, (struct sockaddr *)&client,
                    sizeof(client)) < 0)
    {
        perror("bind() failed");
        close(listen_client);
        exit(-1);
    }

    /*
    * Eschuchar
    */
    if ((rc = listen(listen_slave, 30)) < 0)
    {
        perror("listen() failed");
        close(listen_slave);
        exit(-1);
    }

    if ((rc = listen(listen_client, 30)) < 0)
    {
        perror("listen() failed");
        close(listen_client);
        exit(-1);
    }

    //inicializaciones registro clientes y esclavos

    reg_slv.fd_server_slave.fd = listen_slave;
    reg_slv.fd_server_slave.events = POLLIN;
    fd_server_client.fd = listen_client;
    fd_server_client.events = POLLIN;


    for (i = 0; i<20 ; i++)
      fds_slaves[i].fd = -1;

    for (i = 0; i<10 ; i++)
      fds_clients[i].fd = -1;
    
    //creación de hilos 

    pthread_create(&threads[0],NULL,(void *)&registering_clients,NULL);
    pthread_create(&threads[1],NULL,(void *)&send_msg_client,NULL);

    //pthread_join(threads[0], (void *) &retorno);

    while(1) sleep(1);
    return 0;
}