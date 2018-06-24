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
struct pollfd fds_slaves[20], fds_clients[10];
int result, nfds_slaves, nfds_clients;
int listen_slave = -1, listen_client = -1;
int timeout = (3 * 60 * 1000);

data t_arg;
data_slave reg_slv;
data_client reg_cli; 

void * registering_slaves(data_slave *pto) 
{
  int rc, current_size, new_sd = -1, i;
  
  while(1)
  {
    if (((rc = poll(fds_slaves, nfds_slaves, timeout)) < 0))
    {
      perror("    poll() failed");
      continue;
    }
    
    if (rc == 0) 
    {
      continue;
    }

    current_size = nfds_slaves;
    
    for (i = 0; i < current_size; i++)
    {
      if(pto->fd_server_slave.revents == 0)
        continue;

      if(pto->fd_server_slave.revents != POLLIN)
      {
        printf("Error revents= %d\n", pto->fd_server_slave.revents);
        break;
      }

      if (pto->fd_server_slave.fd == listen_slave)
      {
        do
        {
          if((new_sd = accept(listen_slave, NULL, NULL)) < 0)
          {
              if (errno != EWOULDBLOCK)
                  perror("accept() failed");
              break;
          }
          fds_slaves[nfds_slaves].fd = new_sd; //tener cuidado cuando se desconecta un esclavo
          fds_slaves[nfds_slaves].events = POLLIN;
          nfds_slaves++;
        }while (new_sd != -1);
      }
    }
  }
}

void * registering_clients(data_client *pto) 
{
  int rc, current_size, new_sd = -1, i;
  
  while(1)
  {
    if (((rc = poll(fds_clients, nfds_clients, timeout)) < 0))
    {
      perror("    poll() failed");
      continue;
    }
    
    if (rc == 0) 
    {
      continue;
    }

    current_size = nfds_clients;
    
    for (i = 0; i < current_size; i++)
    {
      if(pto->fd_server_client.revents == 0)
        continue;

      if(pto->fd_server_client.revents != POLLIN)
      {
        printf("Error revents= %d\n", pto->fd_server_client.revents);
        break;
      }

      if (pto->fd_server_client.fd == listen_client)
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
        }while (new_sd != -1);
      }
    }
  }
}

void * send_msg_client(data_client *pto) 
{
  int fd, rc, len, close_conn, i, j;
  int end_server = 0, compress_array = 0;
  char buffer[200];
  
  do
  {
    while(1)
    {
      if(nfds_clients >= 1)
      {
        close_conn = 0;
        poll(fds_clients, nfds_clients, timeout);
        printf("Select a file descriptor: \n");
        fflush(stdin);
        scanf("%d", &fd);
        printf("fd: %d Descriptor selected:  %d revents: %d\n", fd, fds_clients[fd].fd, fds_clients[fd].revents);

        if(fds_clients[fd].fd == -1){ // fd >= nfds
            printf("fd is not in use\n");
            continue;
        }

        if(fds_clients[fd].revents == 0)
        {
            printf("Nothing to read.\n");
            continue;
        }

        if(fds_clients[fd].revents != POLLIN)
        {
            printf("Error revents  padre= %d\n", fds_clients[fd].revents);
            close_conn = 1;
        }else
        {
          // we know that fds[fd].revents == POLLIN
          printf("    Descriptor %d is readable\n", fds_clients[fd].fd);

          rc = recv(fds_clients[fd].fd, buffer, sizeof(buffer), 0);

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

          rc = send(fds_clients[fd].fd, "hola", 16, 0);
          if (rc < 0)
          {
            perror("    send() failed");
            close_conn = 1;
            break;
          }
        }
        
        if (close_conn)
        {
          close(fds_clients[fd].fd);
          compress_array = 1;
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
      }
    }
  }while(end_server == 0);
}

void * send_msg_slave(data_slave *pto) 
{
  int fd, rc, len, close_conn, i, j;
  int end_server = 0, compress_array = 0;
  char buffer[200];
  
  do
  {
    while(1)
    {
      if(nfds_slaves >= 1)
      {
        close_conn = 0;
        poll(fds_slaves, nfds_slaves, timeout);
        printf("Select a file descriptor: \n");
        fflush(stdin);
        scanf("%d", &fd);
        printf("fd: %d Descriptor selected:  %d revents: %d\n", fd, fds_slaves[fd].fd, fds_slaves[fd].revents);

        if(fds_slaves[fd].fd == -1){ // fd >= nfds
            printf("fd is not in use\n");
            continue;
        }

        if(fds_slaves[fd].revents == 0)
        {
            printf("Nothing to read.\n");
            continue;
        }

        if(fds_slaves[fd].revents != POLLIN)
        {
            printf("Error revents  padre= %d\n", fds_slaves[fd].revents);
            close_conn = 1;
        }else
        {
          // we know that fds[fd].revents == POLLIN
          printf("Descriptor %d is readable\n", fds_slaves[fd].fd);

          rc = recv(fds_slaves[fd].fd, buffer, sizeof(buffer), 0);

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

          rc = send(fds_slaves[fd].fd, "hola", 16, 0);
          if (rc < 0)
          {
            perror("    send() failed");
            close_conn = 1;
            break;
          }
        }
        
        if (close_conn)
        {
          close(fds_slaves[fd].fd);
          compress_array = 1;
        }
        
        if (compress_array)
        {
          for (i = 0; i < nfds_slaves; i++)
          {
            if (fds_slaves[i].fd == -1)
            {
              for(j = i; j < nfds_slaves; j++)
              {
                  fds_slaves[j].fd = fds_slaves[j+1].fd;
              }
              i--;
              nfds_slaves--;
              fds_slaves[nfds_slaves].fd = -1;
            }
          }
        }
      }
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

    nfds_clients = 1;
    nfds_slaves = 1;

    reg_slv.fd_server_slave.fd = listen_slave;
    reg_slv.fd_server_slave.events = POLLIN;
    reg_cli.fd_server_client.fd = listen_client;
    reg_cli.fd_server_client.events = POLLIN;


    for (i = 0; i<20 ; i++)
      fds_slaves[i].fd = -1;

    for (i = 0; i<10 ; i++)
      fds_clients[i].fd = -1;
    
    //creación de hilos 

    reg_cli.id = 0;
    pthread_create(&threads[0],NULL,(void *)&registering_clients,(void *)&reg_cli);
    reg_cli.id = 1;
    pthread_create(&threads[1],NULL,(void *)&send_msg_client,(void *)&reg_cli);
    reg_slv.id = 2;
    pthread_create(&threads[2],NULL,(void *)&registering_slaves,(void *)&reg_slv);
    reg_slv.id = 3;
    pthread_create(&threads[3],NULL,(void *)&send_msg_slave,(void *)&reg_slv);
    
    return 0;
}