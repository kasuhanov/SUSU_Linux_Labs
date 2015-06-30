#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h> 
#include <pthread.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>

#define POWER 10000

pthread_mutex_t barMutex;
sem_t * barSem[2];

class Barrier
{
	private:
		int totalThreads;
		int countReady;
		int currentSem;
		int * currentSemArray;

	public:
		Barrier(int inpThreads)
		{
			totalThreads = inpThreads;
			countReady = 0;
			currentSem=0;
			currentSemArray=new int[totalThreads];
		}


		void Wait(int id)
		{
			pthread_mutex_lock(&barMutex);
			countReady++;
			if (countReady<totalThreads)
			{
				currentSemArray[id]=currentSem;
				pthread_mutex_unlock(&barMutex);
				
				sem_wait(barSem[currentSemArray[id]]);	
			}
			else
			{
				printf("SYNCHRONIZED with %3d being the last thread to finish.\n",id);
				int i;
				countReady=0;
				for (i=1;i<totalThreads;i++)
				{
					sem_post(barSem[currentSem]);
				}
				currentSem=(currentSem+1)%2;
				pthread_mutex_unlock(&barMutex);
			}
			return;	
		}		
};

struct BufStruct
{
	int id;
	Barrier* pBar;
};

void* singleint(void* inpBuf)
{
	Barrier* Bar=((BufStruct*)inpBuf)->pBar;
	int id = ((BufStruct*)inpBuf)->id;
	int yolt;
	for (yolt=0; yolt<3;yolt++)
	{
		printf("START thread %3d\n",id);
		int G=0;
		int j=0;
		srand(rand());
		unsigned int r = rand();
		int iters = (1 + rand_r(&r) % 100) * POWER;
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
		float res = 4*G/1.0/iters;
		printf("WAIT  thread %3d (after %7d iterations)\n",id,iters);
		Bar->Wait(id);
		printf("DONE  thread %3d. Result: %3.5f\n",id,res);	
	}
	return NULL;
}




int main(int argc, char** argv)
{
	if (argc == 2)
	{
		int threadCounter = atoi(argv[1]);

		printf("%i threads\n",threadCounter);
    		
		int i=0;

		pthread_mutex_init(&barMutex,NULL);
		

		sem_unlink("bar_sync_semaphore_1");
		
		barSem[0]=sem_open("bar_sync_semaphore_1",O_CREAT, 0777, 0);
		sem_unlink("bar_sync_semaphore_2");
		
		barSem[1]=sem_open("bar_sync_semaphore_2",O_CREAT, 0777, 0);
		

		pthread_t thread[threadCounter];

		Barrier B =Barrier(threadCounter);

		BufStruct *arg = (BufStruct*) malloc(threadCounter*sizeof(BufStruct));

		
		for (i=0;i<threadCounter;i++)
		{		
			arg[i].pBar=&B;
			arg[i].id=i+1;
			if (pthread_create(&thread[i], NULL, singleint, &arg[i]) != 0)
			{
				printf("FAIL");
				perror("fail");
			}
		}



		for (i=0;i<threadCounter;i++)
		{
			pthread_join(thread[i], NULL);
		}
	

		pthread_mutex_destroy(&barMutex);


		sem_close(barSem[0]);
		sem_close(barSem[1]);
		sem_unlink("bar_sync_semaphore_1");
		sem_unlink("bar_sync_semaphore_2");
		printf("END\n");
	}
	return 0;
}

