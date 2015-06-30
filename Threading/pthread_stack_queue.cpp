#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h> 
#include <pthread.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>

pthread_mutex_t stackMutex;
sem_t* stackSem;

struct StackNode
{
	StackNode * next;
	int value;
};

class Stack
{
	private: StackNode * top;
	private: int depth;
	
	public: Stack()
	{
		depth = 0;
		top = NULL;
	}

	public: void Push(int inpValue)
	{
		pthread_mutex_lock(&stackMutex);
		StackNode * temp = new StackNode;
		temp->value = inpValue;
		temp->next = top;
		top = temp;
		depth++;
		sem_post(stackSem);
		pthread_mutex_unlock(&stackMutex);
		return;
	}
	
	public: int Pop()
	{
		sem_wait(stackSem);
		pthread_mutex_lock(&stackMutex);
		if (depth>0)
		{		
			int temp = top->value;
			top = top->next;
			depth--;
			pthread_mutex_unlock(&stackMutex);		
			return temp;
		}
	}
} stack;


pthread_mutex_t queueMutex;
sem_t* queueSem;

struct QueueNode
{
	QueueNode * next;
	int value;
};

class Queue
{
	private: QueueNode * first;
	private: QueueNode * last;
	private: int length;
	
	public: Queue()
	{
		length = 0;
		first = NULL;
		last = NULL;
	}

	public: void Push(int inpValue)
	{
		pthread_mutex_lock(&queueMutex);
		QueueNode * temp = new QueueNode;
		temp->value = inpValue;
		temp->next = NULL;
		length++;
		if (length==1)
		{
			first=temp;
			last=temp;
		}
		else
		{
			last->next=temp;
			last=temp;
		}
		sem_post(queueSem);
		pthread_mutex_unlock(&queueMutex);
		return;
	}
	
	public: int Pop()
	{
		sem_wait(queueSem);
		pthread_mutex_lock(&queueMutex);
		if (length>0)
		{		
			int temp = first->value;
			first = first->next;
			length--;
			pthread_mutex_unlock(&queueMutex);		
			return temp;
		}
	}
} queue;

pthread_mutex_t threadMutex;

void* singleint(void* inpI)
{
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
	printf("CHILD SENT TO STACK AND QUEUE: %i out of %i\n",G,iters);
	pthread_mutex_lock(&threadMutex);
	stack.Push(G);
	queue.Push(G);
	pthread_mutex_unlock(&threadMutex);
	pthread_exit((void*)1);	
}




int main(int argc, char** argv)
{
	if (argc == 3)
	{
		int threadCounter = atoi(argv[1]);
		int iters = atoi(argv[2]);
		printf("%i threads %i iterations per each\n",threadCounter,iters);
    		
		int stackGG=0;
		int queueGG=0;

		int i=0;
		pthread_mutex_init(&threadMutex,NULL);
		pthread_mutex_init(&stackMutex,NULL);
		pthread_mutex_init(&queueMutex,NULL);

		sem_unlink("int_stack_semaphore");
		sem_unlink("int_queue_semaphore");

		stackSem=sem_open("int_stack_semaphore",O_CREAT, 0777, 0);
		queueSem=sem_open("int_queue_semaphore",O_CREAT, 0777, 0);

		pthread_t thread[threadCounter];
		int *arg = (int *) malloc(sizeof(int));
		
		*arg=iters;
		for (i=0;i<threadCounter;i++)
		{
			if (pthread_create(&thread[i], NULL, singleint, arg) != 0)
			{
				perror("fail");
			}
		}

		int stackit;
		int queueit;

		for (i=0;i<threadCounter;i++)
		{
			stackit=stack.Pop();
			stackGG += stackit;
			printf("PARENT RECIEVED FROM STACK: %i out of %i TOTAL: %i out of %i\n",stackit,iters,stackGG,iters*(i+1));
			queueit=queue.Pop();
			queueGG += queueit;
			printf("PARENT RECIEVED FROM QUEUE: %i out of %i TOTAL: %i out of %i\n",queueit,iters,queueGG,iters*(i+1));

			
		}
		

		for (i=0;i<threadCounter;i++)
		{
			pthread_join(thread[i], NULL);
		}

		float stackres = 4*stackGG/1.0/(iters*threadCounter);
		printf("\nTOTAL RESULT FROM STACK: %3.5f\n",stackres);	
		float queueres = 4*queueGG/1.0/(iters*threadCounter);
		printf("\nTOTAL RESULT FROM QUEUE: %3.5f\n",queueres);	

		pthread_mutex_destroy(&stackMutex);
		pthread_mutex_destroy(&queueMutex);
		pthread_mutex_destroy(&threadMutex);

		sem_close(stackSem);
		sem_close(queueSem);
		sem_unlink("int_stack_semaphore");
		sem_unlink("int_queue_semaphore");
	}
	return 0;
}

