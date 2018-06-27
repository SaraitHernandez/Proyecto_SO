#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>

char buffer[200];
int menu();
void selection(int );

int main(int argc, char **argv)
{

  if(argc<2)
  {
    printf("[host] [puerto]\n");
    return 1;
  }
  
  int puerto, conexion;

  
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

    selection(menu());

    n= send(conexion, buffer, 200, 0);
    if(n < 0)
      printf("error send\n");

    n = recv(conexion, buffer, 200, 0);
    if(n < 0)
      printf("error recv\n");

    printf("\n-> Respuesta: %s \n", buffer);
    sleep(4);    
  }
  return 0;
}

int menu()
{
  int op, error;
  do
  {
    system("clear");
    error = 0;
    printf("\nSeleccione el programa que desea correr \n");
    printf("\t 1 -> Piezas recibidas, fábrica ensambladora\n");
    printf("\t 2 -> Cálculo de la conjetura de Collatz por hilos\n");
    printf("\t 3 -> Cálculo de la conjetura de Collatz por procesos\n");
    printf("\t 4 -> Cálculo serie de Fibonacci\n");
    printf("\t Elección: ");
    scanf("%d", &op);
    if(op != 1 && op != 2 && op != 3 && op != 4)
      printf("Elija una opción válida\n"), error = 1, sleep(2);
  }while (error == 1);
  
  return op;
}


void selection(int op)
{
  int slave, in;
  system("clear");

  switch(op)
  {
    case 1:
      printf("\n* Piezas recibidas por la fábrica ensambladora\n");
      printf("\n-> Ingrese la cantidad de paquetes a enviar: ");
      scanf("%d", &in);
      break;
    case 2:
      printf("\n* Cáculo de la conjetura de Collatz por hilos\n");
      printf("\n-> Ingrese un número: ");
      scanf("%d", &in);
      break;
    case 3:
      printf("\n* Cáculo de la conjetura de Collatz por procesos\n");
      printf("\n-> Ingrese un número: ");
      scanf("%d", &in);
      break;
    case 4:
      printf("\n* Cálculo de la Serie de Fibonacci\n");
      printf("\n-> Ingrese un número: ");
      scanf("%d", &in);
      break;
  } 
  
  printf("\n* Seleccione un número de trabajador \nque ejecute por ud el programa: ");
  fflush(stdout);
  scanf("%d", &slave);

  snprintf(buffer, sizeof(buffer), "%d %d %d", slave, op, in);

}