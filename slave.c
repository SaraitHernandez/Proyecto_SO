#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>

int main(int argc, char **argv)
{

  if(argc<2)
  {
    printf("[host] [puerto]\n");
    return 1;
  }
  
  int puerto, conexion;
  char buffer[200];
  
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

  char *a, *cl, *fn, msg[200], *tipo;
  int n, result = 5;
  a = inet_ntoa(cliente.sin_addr);
  printf("Conexion con -> %s:%d\n", a,htons(cliente.sin_port));
  
  while(1)
  {
    n = recv(conexion, buffer, 200, 0);
    if(n < 0)
      printf("error recv\n");
    else
    {
      tipo =  strtok(buffer, " ");
      if (strcmp(tipo, "0") == 0)
      {
        fn =  strtok(NULL, " ");
        cl =  strtok(NULL, " ");
        printf("fd_client[%s], función a correr: %s\n",cl, fn);
      }else
        printf("%s\n",buffer);
    }

    snprintf(msg, sizeof(msg), "%s %d ", cl, result);

    n= send(conexion, msg, 200, 0);
    if(n < 0)
      printf("error send\n");
  }
  return 0;
}
