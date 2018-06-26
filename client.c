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

  char *a, *sl;
  int n;
  a = inet_ntoa(cliente.sin_addr);
  
  system("clear");
  printf("Bienvenido está conectado con -> %s:%d\n", a,htons(cliente.sin_port));
 
  while(1)
  {
    printf("Elija un esclavo \n");
    scanf("%s", sl);
    strcpy(buffer, sl);

    n= send(conexion, buffer, 200, 0);
    if(n < 0)
      printf("error send\n");

    n = recv(conexion, buffer, 200, 0);
    if(n < 0)
      printf("error recv\n");

    printf("respuesta: %s \n", buffer);
    sleep(1);    
  }
  return 0;
}

void menu()
{
  printf("\n Seleccione el programa que desea correr \n");
  printf("\t 0 -> Para usar el sumador\n");
  printf("\t 1 -> Para la fábrica ensambladora\n");
  printf("\t 2 -> Cálculo de la conjetura de Collatz por hilos\n");
  printf("\t 3 -> Cálculo de la conjetura de Collatz por procesos\n");
  printf("\t 4 -> Fibonacci\n");
}


 int operation(int op)
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
 }