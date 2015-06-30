#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h> 
#include <pthread.h>

void* singlehello(int currentNo)
{
	printf("THREAD #%i: HELLO WORLD.\n",currentNo);
	return NULL;
}


int main(int argc, char** argv)
{
	if (argc == 2)
	{
		int threadCounter = atoi(argv[1]);
		printf("%i threads:\n",threadCounter);

		pthread_t thread[threadCounter];
		for (int i=0;i<threadCounter;i++)
		{
			pthread_create(&thread[i],NULL,singlehello,i);
		}
		for (int i=0;i<threadCounter;i++)
		{
			pthread_join(thread[i], NULL);
		}
		printf("\nMain: GOOD BYE WORLD.\n");
	}
	return 0;
}
