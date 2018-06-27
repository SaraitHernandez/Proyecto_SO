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

#define NRO_THR 4


pthread_t threads[NRO_THR];
struct pollfd fds_slaves[20], fds_clients[10], fd_server_client, fd_server_slave;
int result, nfds_slaves = 0, nfds_clients = 0;
int listen_slave = -1, listen_client = -1;
int timeout = (4 * 1000);

void * registering_clients();
void * registering_slaves();
void * send_msg_client();
void * send_msg_slave();
void exit_signal(int);


int main(int argc, char **argv)
{
    if(argc<3)
    {
        printf("%s [port for the slave] [port for the client]\n",argv[0]);
        return 1;
    }
    
    int port_slave, port_client, rc, on = 1, i;
    int *retorno;
    struct sockaddr_in client, slave;
    port_slave = atoi(argv[1]);
    port_client = atoi(argv[2]);
    signal(2, exit_signal);


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
    system("clear");

    fd_server_slave.fd = listen_slave;
    fd_server_slave.events = POLLIN;
    fd_server_client.fd = listen_client;
    fd_server_client.events = POLLIN;

    for (i = 0; i<20 ; i++)
      fds_slaves[i].fd = -1;

    for (i = 0; i<10 ; i++)
      fds_clients[i].fd = -1;
    
    //creación de hilos 

    pthread_create(&threads[0],NULL,(void *)&registering_clients,NULL);
    pthread_create(&threads[1],NULL,(void *)&send_msg_client,NULL);
    pthread_create(&threads[2],NULL,(void *)&registering_slaves,NULL);
    pthread_create(&threads[3],NULL,(void *)&send_msg_slave,NULL);

    pthread_join(threads[0], (void *) &retorno);
    pthread_join(threads[1], (void *) &retorno);
    pthread_join(threads[2], (void *) &retorno);
    pthread_join(threads[3], (void *) &retorno);

    return 0;
}

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
        fds_clients[nfds_clients].fd = new_sd;
        fds_clients[nfds_clients].events = POLLIN;
        nfds_clients++;
      }while (new_sd != -1);
    }
  }
  
}

void * registering_slaves() 
{
  int rc, new_sd = -1, i;
  int nfds = 1;
  while(1)
  {
    if (((rc = poll(&fd_server_slave, nfds, timeout)) < 0))
    {
      perror("poll() failed");
      continue;
    }
    
    if (rc == 0) //timeout
      continue;

    if (fd_server_slave.fd == listen_slave)
    {
      do
      {
        if((new_sd = accept(listen_slave, NULL, NULL)) < 0)
        {
            if (errno != EWOULDBLOCK)
                perror("accept() failed");
            break;
        }
        fds_slaves[nfds_slaves].fd = new_sd;
        fds_slaves[nfds_slaves].events = POLLIN;
        nfds_slaves++;
      }while (new_sd != -1);
    }
  }
  
}

void * send_msg_client() 
{
  int current_size, rc, len, close_conn, i, j, fd;
  int compress_array = 0;
  char buffer[200], msg[200], *sl, *op, *in;

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
      
      if (rc == 0) //Timeout
        continue;
  
      current_size = nfds_clients;

      for (i = 0; i < current_size; i++)
      {
        if(fds_clients[i].revents == 0) //Nothing to read.
          continue;
        
        if(fds_clients[i].revents != POLLIN)
        {
            printf("Error revents = %d\n", fds_clients[i].revents);
            close_conn = 1;
            break;
        }

        if (fds_clients[i].fd == listen_client)
          continue;
        else 
        {
          printf("Evento del cliente: %d\n", fds_clients[i].fd);

          rc = recv(fds_clients[i].fd, buffer, sizeof(buffer), 0);

          if (rc < 0)
          {
              if (errno != EWOULDBLOCK)
                perror("recv() failed"), close_conn = 1;
              break;
          }

          if (rc == 0)
          {
              printf("Connection closed\n"), close_conn = 1;
              break;
          }

          sl = strtok(buffer, " ");
          op = strtok(NULL, " ");
          in = strtok(NULL, " ");
          fd = atoi(sl);

          snprintf(msg, sizeof(msg), "0 %s %s %d ", op, in, fds_clients[i].fd);

          if (fds_slaves[fd].fd != -1)
          {
            rc = send(fds_slaves[fd].fd, msg, 200, 0);
            if (rc < 0)
            {
              perror("send() failed");
              close_conn = 1;
              break;
            }
          }else
            send(fds_clients[i].fd,"slave not available", 50, 0);
        }
      }
   
      if (close_conn)
      {
        fds_clients[i].fd = -1;
        close(fds_clients[i].fd);
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
}

void * send_msg_slave() 
{
  int current_size, rc, len, close_conn, i, j, fd;
  int compress_array = 0, env = 0;
  char buffer[200], *cl, *r;

  while(1)
  {
    if(nfds_slaves >= 1)
    {
      close_conn = 0;

      if ((rc = poll(fds_slaves, nfds_slaves, timeout)) < 0)
      {
        perror("poll() failed");
        continue;
      }
      
      if (rc == 0) //Timeout
        continue;
  
      current_size = nfds_slaves;

      for (i = 0; i < current_size; i++)
      {
        if(fds_slaves[i].revents == 0) //Nothing to read.
          continue;
        
        if(fds_slaves[i].revents != POLLIN)
        {
            printf("Error revents = %d\n", fds_slaves[i].revents);
            close_conn = 1;
            break;
        }

        if (fds_slaves[i].fd == listen_slave)
          continue;
        else 
        {
          printf("Evento del esclavo: %d\n", fds_slaves[i].fd);
          bzero((char *)&buffer, sizeof(buffer));
          rc = recv(fds_slaves[i].fd, buffer, sizeof(buffer), 0);

          if (rc < 0)
          {
              if (errno != EWOULDBLOCK)
                perror("recv() failed"), close_conn = 1;
              break;
          }

          if (rc == 0)
          {
              printf("Connection closed\n"), close_conn = 1;
              break;
          } 

          cl =  strtok(buffer, " ");
          r =  strtok(NULL, " ");
          fd = atoi(cl);
          env = 0;

          for(j=0; j<nfds_clients; j++)
          {
            if (fds_clients[j].fd == fd)
            {
              rc = send(fds_clients[j].fd, r, 200, 0);
              if (rc < 0)
              {
                perror("send() failed");
                close_conn = 1;
                break;
              }
              env = 1;
              break; 
            }
          }
          
          if(!env)
            send(fds_slaves[i].fd,"client not available", 50, 0);
         

        }
      }
   
      if (close_conn)
      {
        fds_slaves[i].fd = -1;
        close(fds_slaves[i].fd);
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
}

void exit_signal(int num)
{
  int i;
 
  for (i = 0; i < nfds_slaves; i++)
  {
    if(fds_slaves[i].fd >= 0)
    {
      send(fds_slaves[i].fd,"1", 20, 0);
      close(fds_slaves[i].fd);
    }
  }
  for (i = 0; i < nfds_clients; i++)
  {
    if(fds_clients[i].fd >= 0)
    {
      send(fds_slaves[i].fd,"1", 20, 0);
      close(fds_clients[i].fd);
    }
  }

  system("clear");
  printf("Hasta luego!\n");
  exit(EXIT_SUCCESS); 
}