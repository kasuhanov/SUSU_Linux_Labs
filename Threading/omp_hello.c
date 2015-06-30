#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h> 


int main(int argc, char** argv)
{
	if (argc == 2)
	{
		int i;
		int threadCounter = atoi(argv[1]);
		printf("%i THREADS:\n",threadCounter);
		#pragma omp parrallel private(i) shared(threadCounter)
		{
			#pragma omp for  
			for (i=0;i<threadCounter;i++)
			{
				printf("THREAD %d: HELLO WORLD!\n",i+1);
			}
		}
		printf("\nMain: GOOD BYE WORLD.\n");
	}
	return 0;
}
