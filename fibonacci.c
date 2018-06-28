#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#define ID 123
#define ROUTE "/bin/lspci"

char buffer[200];

typedef struct
{
  long id;
  long long numero;
}msg_data;

void exit_signal(int);

int main(int argc, char *argv[])
{
	system("clear");
	printf("\nSucesión de Fibonacci\n\n");

	key_t key_colamsg = ftok(ROUTE, ID);
	int id_colamsg, n, cont = 0;
	msg_data data;
	pid_t process;
	signal(2, exit_signal);
	char num[10];

	n = atof(argv[1]) + 1;

	if((id_colamsg = msgget(key_colamsg, 0)) != -1) 
		msgctl(id_colamsg, IPC_RMID, 0);

	if((id_colamsg = msgget(key_colamsg, IPC_CREAT | 0666)) == -1) 
	{
		perror("msgget");
		exit(EXIT_FAILURE);
	}

	process = fork();

	if(process)
	{
		int anterior0 = 0;
		int anterior1 = 1;
		int actual = 0;
		int i=3;

		fflush(stdout);  
		data.id = 1;
		data.numero = 0; 

		if (msgsnd(id_colamsg, (void *)&data, sizeof(msg_data), 0) == -1) 
			perror("Envio fallido");
		
		fflush(stdout);  
		data.id = 2;
		data.numero = 1; 
		if (msgsnd(id_colamsg, (void *)&data, sizeof(msg_data), 0) == -1) 
			perror("Envio fallido");

		while(cont != n)
		{
			actual = anterior0 + anterior1;
			anterior0=anterior1;
			anterior1=actual;
			i++;
			
			if(actual%2  == 0)
				data.id = 1;
			else
				data.id = 2;
			
			data.numero = actual; 
			if (msgsnd(id_colamsg, (void *)&data, sizeof(msg_data), 0) == -1) 
				perror("Envio fallido");
			cont++;
			sleep(1);
		}
	}else{ 
	  while(cont != n)
	  {
			if(msgrcv(id_colamsg, (void *)&data, sizeof(msg_data), 0, 0) == -1) 
			{
			  perror("error\n");
			}else{ 
				snprintf(num, sizeof(num), "%lld ", data.numero);
				strcat(buffer, num);
			  fflush(stdout);
			}
			cont++;
	  } 
}
	return 0; 
}

void exit_signal(int num_signal)
{

	key_t key_colamsg = ftok(ROUTE, ID);
	int id_colamsg;
	msg_data data;

	if((id_colamsg  = msgget(key_colamsg, 0)) != -1) 
		msgctl(key_colamsg, IPC_RMID, 0);

	fflush(stdout);  
	exit(EXIT_SUCCESS); 
}

