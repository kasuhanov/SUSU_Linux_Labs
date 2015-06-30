#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>

int main(int argc, char** argv) {
	
	if (argc == 3) {
			int procs = atoi(argv[1]);
			int iters = atoi(argv[2]);	
			//printf("%i\n", procs);
			//printf("%i\n", iters);
			int n = 0, help;
			int j;
			int pipefd[2];
//
			if (pipe(pipefd) == -1) {
				perror("pipe");
				exit(EXIT_FAILURE);
			}
//
			for (j = 0; j < procs; j++) {
				pid_t cpid = fork();
				if (cpid == -1) {
					perror("fork");
					exit(EXIT_FAILURE);
				}
				if (cpid == 0) { //child
					close(pipefd[0]);
					int i = 0;
					int k = 0;
					printf("calcing %d iters\n", iters/procs);
					for (i = 0; i < iters/procs; i++) {
						float x = rand() / 1.0 / RAND_MAX;
						float y = rand() / 1.0 / RAND_MAX;
						if (x * x + y * y < 1) {
							k++;
						}
					}
					fflush(stdout);
					write(pipefd[1], &k, sizeof(int));
					close(pipefd[1]);
					_exit(0);
				} 
			}
			close(pipefd[1]);
			for (j = 0; j < procs; j++) {
				read(pipefd[0], &help, sizeof(int));
			    n += help;
			}
			close(pipefd[0]);
			float pi = 4.0 * n / iters;
			printf("PI is %1.5f \n", pi);				

			return 0;
	}
	else return -1;
}
