#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
int main(int argc, char** argv)
{
  if (argc == 3)
  {
    int procs = atoi(argv[1]);
    int iters = atoi(argv[2]);
    //printf("%i %i",procs,iters);
    
    int GG=0;
    int i=0;
    int pipefd[2];
    if (pipe(pipefd) == -1)
    {
      perror("pipe");
      exit(EXIT_FAILURE);
    }    
    for (i = 0; i<procs;i++)
    {
      pid_t cpid=fork();
      if (cpid == 0)
      {
        close(pipefd[0]);
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
        //printf("CHILDSENT: %i\n",G);
       
	
        write(pipefd[1],&G,sizeof(G));
        close(pipefd[1]);
        _exit(0);
      }
      else
      {
        int chST;
        waitpid(cpid, &chST,0);  
      }
    }
    close(pipefd[1]);
    int it=0;
    for (i = 0; i < procs; i++)
    {

      read(pipefd[0], &it, sizeof(it));
      GG += it;
      //printf("PARENTRECIEVED: %i NOW: %i\n",it,GG);
    }
    close(pipefd[0]);
    float res = 4*GG/1.0/(iters*procs);
    printf("\n%3.5f\n",res);				
    return 0;
  }
}
