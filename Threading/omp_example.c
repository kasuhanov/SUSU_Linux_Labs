//-fopenmp
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h> 
#include <omp.h>

int main(int argc, char** argv)
{
	if (argc == 3)
	{
		int threadCounter = atoi(argv[1]);
		int iters = atoi(argv[2]);
		printf("%i threads %i iterations per each\n",threadCounter,iters);
    
		int GG=0;
		int i=0;
		int G;
		int j;
		float x;
		float y;
		int ran;
		#pragma omp parallel private(i,G,j,x,y,ran) shared(GG,iters)
		{
			#pragma omp for
			for (i=0;i<threadCounter;i++)
			{
				G=0;
				j=0;
				srand(rand());
				ran = rand();
				for (j=0; j<iters; j++)
				{
					x=rand_r(&ran)/1.0/RAND_MAX;
					y=rand_r(&ran)/1.0/RAND_MAX;
					if (x*x+y*y<1)
					{
						G++;
					}
				}
				fflush(stdout);
				printf("CHILD SENT: %i out of %i\n",G,iters);
				#pragma omp atomic
				GG+=G;
			}
		
		}
		float res = 4*GG/1.0/(iters*threadCounter);
		printf("\nTOTAL RESULT: %3.5f\n",res);				
	}
	return 0;
}
