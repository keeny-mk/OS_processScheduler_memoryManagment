#include "headers.h"
 processData data[max];
enum Algorithm algorithm;

void clearResources(int);

int main(int argc, char * argv[])
{
    heap_t * sorted_p = (heap_t *)calloc(1, sizeof (heap_t));

    signal(SIGINT, clearResources);
    // TODO Initialization
    // 1. Read the input files.
    int i=0;
    FILE *pFile;
    pFile=fopen("processes.txt","r");
    if(pFile!=NULL)
    {
    char firstLine[100];
    fgets(firstLine, 100, pFile);
    int c=1;
    //= getc(pFile);


while(c!=EOF)
    {
    c=fscanf(pFile,"%d %d %d %d %d",&data[i].id,&data[i].arrivaltime,&data[i].runningtime,&data[i].priority,&data[i].mem);
    if (c==EOF) break;

    pushH(sorted_p,data[i].arrivaltime, &data[i]);
    printf("%d %d %d %d %d\n",data[i].id,data[i].arrivaltime,data[i].runningtime,data[i].priority,data[i].mem);
    i++;
   // c=getc(pFile);
    }
    fclose(pFile);
    }
    else
    {
    printf("Error Reading Input File. \n");
    }

    /*
int l=sorted_p->len;
    processData* temp;
    for(int j=0; j<l; j++)
    {
    temp=pop(sorted_p);
    printf("%d %d %d %d\n",temp->id,temp->arrivaltime,temp->runningtime,temp->priority);
    }
    //testing the priority queue
*/

    // 2. Ask the user for the chosen scheduling algorithm and its parameters, if there are any.
    printf("Enter The Required Schedueling Algorithm ('0->RR'(with quanta 5 second),'1->HPF','2->SJF (premptive)') \n");
    int input;
    scanf("%d",&input);
    printf("The Required Algorithm is %d \n",input);
    int shmid_alg = shmget(555, 8, IPC_CREAT | 0744);
    if ((long)shmid_alg == -1)
    {
        perror("Error in creating shm!");
        exit(-1);
    }
    int * shmaddr_alg = (int *) shmat(shmid_alg, (void *)0, 0);
    if ((long)shmaddr_alg == -1)
    {
        perror("Error in attaching the algorithm in memory for scheduler!");
        exit(-1);
    }
    *shmaddr_alg = input;
    //algorithm=(enum Algorithm)input;
   // printf("%d \n ",(int)algorithm);

   //new: termination
   int shmid_len=shmget(666,8,IPC_CREAT|0744);
   if ((long)shmid_len== -1)
    {
        perror("Error in creating shm_len!");
        exit(-1);
    }
    int * shmaddr_len = (int *) shmat(shmid_len, (void *)0, 0);
    if ((long)shmaddr_len == -1)
    {
        perror("Error in attaching the processes length in memory for scheduler!");
        exit(-1);
    }
    *shmaddr_len = sorted_p->len;
    //end new

    // 3. Initiate and create the scheduler and clock processes.
    printf("\n process generator pid = %d\n", getpid());

    int pid1=fork();
    if(pid1==-1)
    {
    printf("Error in first Forking /n");
    }
    else if (pid1==0)
    {
    printf("\nI am the clock child, my pid = %d and my parent's pid = %d\n\n", getpid(), getppid());
    char *args[]={"./clk.out",NULL};
        execvp(args[0],args);
    //Clock Process is here
    }

    //Generator Continues here after forking clock
    int pid2=fork();
     if(pid2==-1)
    {
    printf("Error in second Forking /n");
    }

    else if (pid2==0)
    {
    printf("\nI am the scheduler child, my pid = %d and my parent's pid = %d\n\n", getpid(), getppid());
    char *args[]={"./scheduler.out",NULL};
        execvp(args[0],args);
    //Schedular process is here
    }
    //Generator continues here after forking schedular

    int ps_id;
    ps_id =msgget(69,IPC_CREAT | 0766);
    if(ps_id == -1)
    {
        perror("Error in p_s msgqueue");
        exit(-1);
    }

    int shmid_ps;
    shmid_ps = shmget(420, 8, IPC_CREAT | 0744);
    int * shmaddr_ps = (int *) shmat(shmid_ps, (void *)0, 0);
    *shmaddr_ps=0;

    // 4. Use this function after creating the clock process to initialize clock
    initClk();
    // To get time use this
    int x,pt;

    while(1)
    {
    processData msgbuff = (*popH(sorted_p));
    //printf("process of id %d and arrival time %d popped\n",msgbuff.id,msgbuff.arrivaltime);
    while(msgbuff.arrivaltime!=x)
    {
        x= getClk();
        if (pt!=x)printf("current time is %d\n", x);
        pt=x;
    }
    int sendVal=msgsnd(ps_id,&msgbuff,sizeof(msgbuff),!IPC_NOWAIT);
    if(sendVal == -1)
    { perror("Send Error");}
    else {
   // printf("process of id %d and arrival time %d sent\n ", msgbuff.id, msgbuff.arrivaltime);
    (*shmaddr_ps)++;
   }

    if (sorted_p->len==0)
    {
    break;
    }
    }


    // TODO Generation Main Loop
    // 5. Create a data structure for processes and provide it with its parameters.
    switch (algorithm)
    {

    case HPF:
    break;

    case SJF:
    break;

    case RR:
    break;


    }

    //new: waiting for scheduler to finish
        int status;

    wait(&status);
    if(WIFEXITED(status))
    printf("\nThe scheduler has terminated with exit code %d\n", WEXITSTATUS(status));
    //end new

   // sleep(300);
    // 6. Send the information to the scheduler at the appropriate time.
    // 7. Clear clock resources
    destroyClk(true);
    msgctl(69,IPC_RMID,0);
shmctl(555,IPC_RMID,0);
shmctl(420,IPC_RMID,0);
shmctl(666,IPC_RMID,0);
}

void clearResources(int signum)
{
msgctl(69,IPC_RMID,0);
shmctl(555,IPC_RMID,0);
shmctl(420,IPC_RMID,0);
shmctl(666,IPC_RMID,0);
    //TODO Clears all resources in case of interruption
}
