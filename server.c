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

int conexionServidor, conexionEsclavo[4];

void signalExit(int );

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

  int puerto, process_pid[4], i=0;
  pid_t process;
  socklen_t longCliente;
  struct sockaddr_in servidor, cliente;
  char buffer[200];
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

  listen(conexionServidor, 10);


  /*
  * Procesos encargados de: escuchar esclavos, escuchar al cliente, proceso encargado de los hijos que terminan
  

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
  
 // if (getpid() == process_pid[1]) 
  //{

    while(i != 4)
    {


      printf("Esperando, puerto: %d\n", ntohs(servidor.sin_port));

      longCliente = sizeof(cliente);
      
      if((conexionEsclavo[i] = accept(conexionServidor, (struct sockaddr *)&cliente, &longCliente)) < 0)
      { 
        printf("Error conexion \n");
        close(conexionServidor);
        return 1;
      }else{
        printf("conexion: %d  esclavo %d\n", i, conexionEsclavo[i]);
        if(send(conexionEsclavo[i],"conectado", 15, 0) < 0) 
        {   
          printf("Error enviando mensaje \n");
          close(conexionServidor);
          return 1;
         
        }
         i++;
      }
    }

    i=0;
    while(i != 4)
    {
        send(conexionEsclavo[i],"msj 2", 15, 0), i++;
    }

//  }


  /*
  * Escuchar cliente 
  
  
  else if (getpid() == process_pid[2]) 
  {

  }
 

  /*
  * Escuchar escalvos que terminan (?) 
  
  
  else if (getpid() == process_pid[3]) 
  {

  }

  /*
  * Padre : espera a todos los hijos terminen, libera recursos y ...(?)
  
  else
  {


  } 

  */
  return 0;  
}


void signalExit(int i) {
    close(conexionEsclavo[0]);
    close(conexionServidor);
    int id_queue;
    key_t key_q = ftok(".", 420);
    if((id_queue = msgget(key_q, 0)) != -1) 
      msgctl(key_q, IPC_RMID, 0);
    exit(EXIT_SUCCESS);
}