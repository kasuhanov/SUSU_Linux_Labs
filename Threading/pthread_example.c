#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h> 
#include <pthread.h>

void* singleint(void* inpI)
{
	int* ret = malloc(sizeof(int));
	int iters=*((int*)inpI);
	int G=0;
	int j=0;
	for (j=0; j<iters; j++)
	{
		float x=rand()/1.0/RAND_MAX;
		float y=rand()/1.0/RAND_MAX;
		if (x*x+y*y<1)
		{
			G++;
		}
	}
	fflush(stdout);
	printf("CHILD SENT: %i out of %i\n",G,iters);
	*ret=G;
	return ret;
	
}


int main(int argc, char** argv)
{
	if (argc == 3)
	{
		int threadCounter = atoi(argv[1]);
		int iters = atoi(argv[2]);
		printf("%i threads %i iterations per each\n",threadCounter,iters);
    
		int GG=0;
		int i=0;

		pthread_t thread[threadCounter];
		int *arg = malloc(sizeof(int));
		int **ret_arg = calloc(threadCounter,sizeof(int*));
		*arg=iters;
		for (i=0;i<threadCounter;i++)
		{
			if (pthread_create(&thread[i], NULL, singleint, arg) != 0)
			{
				perror("fail");
			}
		}

		int it;

		for (i=0;i<threadCounter;i++)
		{
			pthread_join(thread[i], (void**) &ret_arg[i]);
			it=*ret_arg[i];
			GG += it;
			printf("PARENT RECIEVED: %i out of %i TOTAL: %i out of %i\n",it,iters,GG,iters*(i+1));
			
		}
		float res = 4*GG/1.0/(iters*threadCounter);
		printf("\nTOTAL RESULT: %3.5f\n",res);				
	}
	return 0;
}
