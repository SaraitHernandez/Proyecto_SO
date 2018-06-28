#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

char buffer[200];

int main(int argc, char *argv[]){
	
	system("clear");
	printf("\n_______________________Conjetura de Collatz_______________________\n\n");
	
	pid_t ProcessChild, w;
	int i, n, wstatus; 
	char num[10];

	n = atof ( argv [1])
	ProcessChild = fork();

	if(!ProcessChild) {
		while(n!=1){
			snprintf(num, sizeof(num), "%d, ", n);
			strcat(buffer, num);
			if (n%2 == 0) 
				n = n/2;
			else
				n = 3*n + 1;
		}
	}

	w = waitpid(ProcessChild, &wstatus, WUNTRACED | WCONTINUED);
    
  if (WIFEXITED(wstatus)) 
    WEXITSTATUS(wstatus);

  printf("%s\n",buffer);
	return 0;
}