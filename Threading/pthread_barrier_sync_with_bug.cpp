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
sem_t * barSem;
sem_t * barReadySem;

class Barrier
{
	private:
		int totalThreads;
		int countReady;
		//int countDone;
		//bool barReady;

	public:
		Barrier(int inpThreads)
		{
			totalThreads = inpThreads;
			countReady = 0;
			//countDone = 0;
			//barReady=true;
		}

		void Sync(int id)
		{
			pthread_mutex_lock(&barMutex);
			countReady++;			
			if (countReady<totalThreads)
			{
				pthread_mutex_unlock(&barMutex);
				if (id==3)
				{
					sleep(3);
				}
				sem_wait(barSem);	
			}
			else
			{
				printf("SYNCHRONIZED with %3d being the last thread to finish.\n",id);
				int i;
				for (i=0;i<totalThreads;i++)
				{
					sem_post(barSem);
				}
				countReady=0;
				
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
		
		if (id==1)
		{
			if (yolt==0) {iters=1;}
			if (yolt==1) {iters=1;}
		}


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
		Bar->Sync(id);
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
		

		sem_unlink("bar_sync_semaphore");
		
		barSem=sem_open("bar_sync_semaphore",O_CREAT, 0777, 0);
		

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


		sem_close(barSem);
		sem_unlink("bar_sync_semaphore");
		printf("END\n");
	}
	return 0;
}

