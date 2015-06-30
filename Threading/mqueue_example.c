#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <mqueue.h>

#define MSGQOBJ_NAME "/myqueue"
#define MAX_MSG_LEN     10000


int main(int argc, char** argv)
{
  if (argc == 3)
  {
    int procs = atoi(argv[1]);
    int iters = atoi(argv[2]);
    printf("%i processes %i iterations per each\n",procs,iters);
    
    int GG=0;
    int i=0;


    int msgsz;
    unsigned int sender;
 

    mqd_t msgq_id;
    unsigned int msgprio = 0;
    pid_t my_pid = getpid();
    char msgcontent[MAX_MSG_LEN];
    int create_queue = 0;
    struct mq_attr msgq_attr;            
    
    mq_unlink("/myqueue");
    for (i = 0; i<procs;i++)
    {
      pid_t cpid=fork();
      if (cpid == 0)//child
      {
        msgq_id = mq_open(MSGQOBJ_NAME, O_RDWR);
   
        if (msgq_id == (mqd_t)-1)
        {
          perror("In mq_open()");
          exit(1);
        }

        
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
        printf("CHILDSENT: %i out of %i\n",G,iters);
       
	snprintf(msgcontent, MAX_MSG_LEN, "%i", G);
        mq_send(msgq_id, msgcontent, strlen(msgcontent)+1, msgprio);

        _exit(0);
      }
      else//parent
      {
        msgq_id = mq_open(MSGQOBJ_NAME, O_CREAT | O_RDWR, 0664,0);

        if (msgq_id == (mqd_t)-1)
        {
          perror("In mq_open()");
          exit(1);
        }
      }
    }
 //no child would get here
    int it=0;
    i=0;
    while (i < procs)
    {
      //while (wait()>0)
      //{
      //}
      mq_getattr(msgq_id, &msgq_attr);
      if (msgq_attr.mq_curmsgs>0)
      {
        msgsz = mq_receive(msgq_id, msgcontent, MAX_MSG_LEN, &sender);
        if (msgsz == -1)
        {
          perror("In mq_receive()");
          exit(1);
        }
        int it = atoi(msgcontent);
        GG += it;
        printf("PARENTRECIEVED: %i out of %i TOTAL: %i out of %i\n",it,iters,GG,iters*(i+1));
	i++;
      }
      
    }
    mq_close(msgq_id);
    
    float res = 4*GG/1.0/(iters*procs);
    printf("\nTOTAL RESULT: %3.5f\n",res);				
    return 0;
  }
}
