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



int conexionServidor, conexionCliente, conexionCliente2;

void signalExit(int i) {
    close(conexionCliente);
    close(conexionServidor);
    int id_queue;
    key_t key_q = ftok(".", 420);
    if((id_queue = msgget(key_q, 0)) != -1) 
      msgctl(key_q, IPC_RMID, 0);
    exit(EXIT_SUCCESS);
}


int main(int argc, char **argv)
{

  signal(SIGINT, signalExit);   // SIGINT
  signal(SIGQUIT, signalExit); // SIGQUIT
  signal(SIGSTOP, signalExit);  // SIGTSTOP
  signal(SIGKILL, signalExit);
     
  if(argc<2)
  {
    printf("%s [puerto]\n",argv[0]);
    return 1;
  }

  int puerto, id_queue, client=1;
  socklen_t longCliente;
  struct sockaddr_in servidor, cliente;
  char buffer[200], buffer2[200];
  puerto = atoi(argv[1]);
  
  /*
  * Asignacion -> socket
  */
  conexionServidor = socket(AF_INET, SOCK_STREAM, 0);

  bzero((char *)&servidor, sizeof(servidor));

  servidor.sin_family = AF_INET;
  servidor.sin_port = htons(puerto);
  servidor.sin_addr.s_addr = INADDR_ANY; //Macro -> Propia Direccion

  /*
  * Asignacion -> socket a puerto
  */
  if(bind(conexionServidor, (struct sockaddr *)&servidor, sizeof(servidor)) < 0)
  {
    printf("Error socket a\n");
    close(conexionServidor);
    return 1;
  }

  /*
  * Escuchando ...
  */
  listen(conexionServidor, 2);

  printf("Esperando, puerto: %d\n", ntohs(servidor.sin_port));

  longCliente = sizeof(cliente);
  
  if((conexionCliente = accept(conexionServidor, (struct sockaddr *)&cliente, &longCliente)) < 0)
  { 
    printf("Error conexion \n");
    close(conexionServidor);
    return 1;
  }else{
    send(conexionCliente, "cliente1", 15, 0); client = 0;
  }

  if((conexionCliente = accept(conexionServidor, (struct sockaddr *)&cliente, &longCliente)) < 0)
  { 
    printf("Error conexion \n");
    close(conexionServidor);
    return 1;
  }else
    send(conexionCliente, " ", 15, 0), printf("aqui");;
  
  /*Envío de mensajes*/

  while(1)
  {
      
    if(client == 0)
    {
      if(recv(conexionCliente, buffer, 200, 0) < 0)
      {
        printf("Error\n");
        close(conexionServidor);
        return -1;
      }else
      {
        send(conexionCliente2, buffer, 200, 0);
        bzero((char *)&buffer, sizeof(buffer));
      }
    }else 
    {
      if(recv(conexionCliente2, buffer, 200, 0) < 0)
      {
        printf("Error\n");
        close(conexionServidor);
        return -1;
      }else{
        send(conexionCliente, buffer, 200, 0);
        bzero((char *)&buffer, sizeof(buffer));
      }
    }        
  }

  return 0;  
}


