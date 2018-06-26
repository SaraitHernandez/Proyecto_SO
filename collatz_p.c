#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int n;
int NProceso;
pid_t ProcessChild = -1;
int main(){

	int *values = (int*)malloc(sizeof(int));
	pid_t *pids = (pid_t*)malloc(sizeof(pid_t));
	int count = 0;
	printf("%s\n", "intruduzca tanto numero como desee, 0 para terminar de introducir.");

	while(1){
		scanf("%d", &n);
		if (n==0) {
			break;
		} else {
			values[count] = n;
			values = (int*)realloc(values, (count+2)* sizeof(int));
			pids = (pid_t*)realloc(pids, (count+2)* sizeof(pid_t));
			count++;
		}
	}


	for (int i = 0; i < count; ++i) {
		if (ProcessChild || ProcessChild == -1){
			ProcessChild = fork();
			pids[i] = ProcessChild;
		}
		if(!ProcessChild) {
			count = 0;
			int n = values[i];
			while(n!=1){
				if (n%2 == 0) {
					n = n/2;
				} else {
					n = 3*n + 1;
				}
				
			}
			printf("%s %d\n", "Se cumple para el numero", values[i]);
			break;
			
		}
	}

	for (int i = 0; i < count; ++i)
	{
		int wstatus;
		pid_t w;
		do 
	    {
	      w = waitpid(pids[i], &wstatus, WUNTRACED | WCONTINUED);
	     
	      if (WIFEXITED(wstatus)) 
	      {
	        WEXITSTATUS(wstatus);
	      } 
	     
	    } while (!WIFEXITED(wstatus) && !WIFSIGNALED(wstatus));
	}
	
	return 0;
}