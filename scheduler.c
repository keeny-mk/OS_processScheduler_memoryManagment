#include "headers.h"
void decRT(processTable* pt)
{
pt->remainingTime--;
int shmid_sp = shmget(pt->pid, 8, 0777);
int *shmaddr_sp = (int *) shmat(shmid_sp, (void *)0, 0);
(*shmaddr_sp)--;
}

void *malloc_mem(size_t request) {
  size_t original_container;
  size_t container;

  if (request + HEADER_SIZE > MAX_ALLOC) {
    return NULL;
  }

  if (bptr == NULL) {
    bptr = mxptr = (uint8_t *)sbrk(0);
    container_limit = CONT_COUNT - 1;
    mxptrNewVal(bptr + sizeof(list_mem));
    makelist(&containers[CONT_COUNT - 1]);
    listPush(&containers[CONT_COUNT - 1], (list_mem *)bptr);
  }

  container = requestContainer(request + HEADER_SIZE);
  original_container = container;

  while (container + 1 != 0) {
    uint8_t *ptr;
    size_t spaceNeed;
    size_t k;
    size_t size;


    if (!GrowTree(container)) {
      return NULL;
    }

    ptr = (uint8_t *)listPop(&containers[container]);
    if (!ptr) {

      if (container != container_limit || container == 0) {
        container--;
        continue;
      }

      if (!GrowTree(container - 1)) {
        return NULL;
      }
      ptr = (uint8_t *)listPop(&containers[container]);
    }

    size = (size_t)1 << (MAX_ALLOC_POW- container);
    if(container<original_container){
        spaceNeed= size/2 + sizeof(list_mem);
    }
    else{
        spaceNeed=size;
    }
    if (!mxptrNewVal(ptr + spaceNeed)) {
      listPush(&containers[container], (list_mem *)ptr);
      return NULL;
    }

    k = MapA2N(ptr, container);
    if (k != 0) {
      FisParentSplit(k);
    }

    while (container < original_container) {
      k= k * 2 + 1;
      container++;
      FisParentSplit(k);
      listPush(&containers[container], (list_mem *)MapN2A(k + 1, container));
    }


    *(size_t *)ptr = request;
    //printf("ptr = %d\n",ptr);
    return ptr + HEADER_SIZE;
  }

  return NULL;
}

void free_mem(void *ptr) {
  size_t container;
  size_t k;
  if (!ptr) {
    return;
  }

  ptr = (uint8_t *)ptr - HEADER_SIZE;
 // printf("pointer in free = %ld\n",(uint8_t*)ptr);
  container = requestContainer(*(size_t *)ptr + HEADER_SIZE);
  k = MapA2N((uint8_t *)ptr, container);

  while (k != 0) {
    FisParentSplit(k);
    printf("is parent split = %d\n",(int)isParentSplit(k));
    if (isParentSplit(k) || container == container_limit) {
      break;
    }
    listRemove((list_mem *)MapN2A(((k - 1) ^ 1) + 1, container));
    k = (k - 1) / 2;
    container--;
  }

  listPush(&containers[container], (list_mem *)MapN2A(k, container));
}

int main(int argc, char * argv[])
{

    printf("Scheduler initialized\n");
   enum Algorithm whichAlgorithm;
    int shmid_algs = shmget(555, 8, 0744);
    int *shmaddr_algs = (int *) shmat(shmid_algs, (void *)0, 0);
    whichAlgorithm=(*shmaddr_algs);
    printf("Scheduler is Working on Algorithm %d \n",*shmaddr_algs);
    initClk();
FILE *pOFile;

//To Erase AnyThing In The File
pOFile=fopen("RR_out.txt","w");
fclose(pOFile);

FILE *HPF_file;
    HPF_file=fopen("HPF_out.txt","w");
    fprintf(HPF_file, "File opened\n");
    fclose(HPF_file);

//new
FILE *SJF_file;
    SJF_file=fopen("SJF_out.txt","w");
    fprintf(SJF_file, "File opened\n");
    fclose(SJF_file);

FILE *perf_file;
    perf_file=fopen("perf_out.txt","w");
    fprintf(perf_file, "File opened\n");
    fclose(perf_file);

FILE *mem_file;
    mem_file=fopen("mem_log.txt","w");
    fprintf(mem_file, "File opened\n");
    fclose(mem_file);


//end new

//new
int shmid_len = shmget(666, 8, 0744);
    int *shmaddr_len = (int *) shmat(shmid_len, (void *)0, 0);
    int len = *shmaddr_len;
//end new


    int ps_id;
    ps_id=msgget(69,0);
    if(ps_id == -1)
    {
        perror("Error in getting p_s msgqueue");
        exit(-1);
    }

    int shmid_ps=shmget(420, 8, 0744);
    int *shmaddr_ps = (int *) shmat(shmid_ps, (void *)0, 0);

    processData schd_pd;
    processTable pTable;

    Node* pTables = NULL;


    int * RT;
    int quanta=10;
int howLongRunning=0;
processTable inTurnProcess;
processTable processTurn;

    int i=1;
    int z=0;

    int fin=0;
    float tot_wta=0;
    int tot_runtime=0;
    int tot_wait=0;
    float wta_arr[10];
    int g=0;

    int curr_time=getClk();
    int prev_time=getClk()-1;
    int TA;
    float WTA;
    int WT;

while(1)
{
    curr_time=getClk();
    int pid;



if((*shmaddr_ps)>0)
{
    int recVal= msgrcv(ps_id,&schd_pd,sizeof(schd_pd),0,!IPC_NOWAIT);
    if(recVal == -1)
    {
    perror("Receive Error");
    break;
    }

pTable.pData=schd_pd;
pTable.remainingTime=pTable.pData.runningtime;
pTable.running=false;
pTable.finished=false;
pid=fork();
  if(pid==-1)
    {
    printf("Error in process Forking /n");
    }
    else if(pid==0)
    {
    char *args[]={"./process.out",NULL};
    execvp(args[0],args);
    }
else
{

pTable.pid=pid;

int shmid_sp;
shmid_sp = shmget(pid, 8, IPC_CREAT | 0744);
pTable.shmID=shmid_sp;
    int * shmaddr = (int *) shmat(shmid_sp, (void *)0, 0);
    *shmaddr=pTable.remainingTime;
if(whichAlgorithm==RR)
    {
   pTable.memaddress=malloc_mem((size_t) pTable.pData.mem);
if(pTable.memaddress!=NULL) {
    printf("At time %d allocated %d bytes for process %d from %ld to %ld\n",getClk(),pTable.pData.mem,pTable.pData.id,((uint8_t *)pTable.memaddress-bptr)-HEADER_SIZE,((uint8_t *)pTable.memaddress-bptr)-HEADER_SIZE+(int)pow(2,ceil(log((double)pTable.pData.mem)/log(2))));
    mem_file=fopen("mem_log.txt","a+");
    fprintf(mem_file,"At time %d allocated %d bytes for process %d from %ld to %ld\n",getClk(),pTable.pData.mem,pTable.pData.id,((uint8_t *)pTable.memaddress-bptr)-HEADER_SIZE,((uint8_t *)pTable.memaddress-bptr)-HEADER_SIZE+(int)pow(2,ceil(log((double)pTable.pData.mem)/log(2))));
    fclose(mem_file);
    }

    enqueue(pTable);
    printf("Process %d arrived at circular queue of RR \n",peekRR().pid);
    }
else if(whichAlgorithm==HPF)
{
if (isEmpty(&pTables)==1)
{
    pTable.memaddress=malloc_mem((size_t) pTable.pData.mem);
    if(pTable.memaddress!=NULL) {
    printf("At time %d allocated %d bytes for process %d from %ld to %ld\n",getClk(),pTable.pData.mem,pTable.pData.id,((uint8_t *)pTable.memaddress-bptr)-HEADER_SIZE,((uint8_t *)pTable.memaddress-bptr)-HEADER_SIZE+(int)pow(2,ceil(log((double)pTable.pData.mem)/log(2))));
    mem_file=fopen("mem_log.txt","a+");
    fprintf(mem_file,"At time %d allocated %d bytes for process %d from %ld to %ld\n",getClk(),pTable.pData.mem,pTable.pData.id,((uint8_t *)pTable.memaddress-bptr)-HEADER_SIZE,((uint8_t *)pTable.memaddress-bptr)-HEADER_SIZE+(int)pow(2,ceil(log((double)pTable.pData.mem)/log(2))));
    fclose(mem_file);
    }
    printf("At time %d allocated %d bytes for process %d from %ld to %ld\n",getClk(),pTable.pData.mem,pTable.pData.id,((uint8_t *)pTable.memaddress-bptr)-HEADER_SIZE,((uint8_t *)pTable.memaddress-bptr)-HEADER_SIZE+(int)pow(2,ceil(log((double)pTable.pData.mem)/log(2))));

    pTables = newNode(pTable, pTable.pData.priority);
}
else
{
 pTable.memaddress=malloc_mem((size_t) pTable.pData.mem);
 if(pTable.memaddress!=NULL) {
    printf("At time %d allocated %d bytes for process %d from %ld to %ld\n",getClk(),pTable.pData.mem,pTable.pData.id,((uint8_t *)pTable.memaddress-bptr)-HEADER_SIZE,((uint8_t *)pTable.memaddress-bptr)-HEADER_SIZE+(int)pow(2,ceil(log((double)pTable.pData.mem)/log(2))));
    mem_file=fopen("mem_log.txt","a+");
    fprintf(mem_file,"At time %d allocated %d bytes for process %d from %ld to %ld\n",getClk(),pTable.pData.mem,pTable.pData.id,((uint8_t *)pTable.memaddress-bptr)-HEADER_SIZE,((uint8_t *)pTable.memaddress-bptr)-HEADER_SIZE+(int)pow(2,ceil(log((double)pTable.pData.mem)/log(2))));
    fclose(mem_file);
    }
 printf("At time %d allocated %d bytes for process %d from %ld to %ld\n",getClk(),pTable.pData.mem,pTable.pData.id,((uint8_t *)pTable.memaddress-bptr)-HEADER_SIZE,((uint8_t *)pTable.memaddress-bptr)-HEADER_SIZE+(int)pow(2,ceil(log((double)pTable.pData.mem)/log(2))));

push(&pTables,pTable,pTable.pData.priority);
}

}
//new
else if(whichAlgorithm==SJF)
{
if (isEmpty(&pTables)==1)
{
pTable.memaddress=malloc_mem((size_t) pTable.pData.mem);
if(pTable.memaddress!=NULL) {
    printf("At time %d allocated %d bytes for process %d from %ld to %ld\n",getClk(),pTable.pData.mem,pTable.pData.id,((uint8_t *)pTable.memaddress-bptr)-HEADER_SIZE,((uint8_t *)pTable.memaddress-bptr)-HEADER_SIZE+(int)pow(2,ceil(log((double)pTable.pData.mem)/log(2))));
    mem_file=fopen("mem_log.txt","a+");
    fprintf(mem_file,"At time %d allocated %d bytes for process %d from %ld to %ld\n",getClk(),pTable.pData.mem,pTable.pData.id,((uint8_t *)pTable.memaddress-bptr)-HEADER_SIZE,((uint8_t *)pTable.memaddress-bptr)-HEADER_SIZE+(int)pow(2,ceil(log((double)pTable.pData.mem)/log(2))));
    fclose(mem_file);
    }
printf("At time %d allocated %d bytes for process %d from %ld to %ld\n",getClk(),pTable.pData.mem,pTable.pData.id,((uint8_t *)pTable.memaddress-bptr)-HEADER_SIZE,((uint8_t *)pTable.memaddress-bptr)-HEADER_SIZE+(int)pow(2,ceil(log((double)pTable.pData.mem)/log(2))));

    pTables = newNode(pTable, pTable.remainingTime);
}
else
{
 pTable.memaddress=malloc_mem((size_t) pTable.pData.mem);
 if(pTable.memaddress!=NULL) {
    printf("At time %d allocated %d bytes for process %d from %ld to %ld\n",getClk(),pTable.pData.mem,pTable.pData.id,((uint8_t *)pTable.memaddress-bptr)-HEADER_SIZE,((uint8_t *)pTable.memaddress-bptr)-HEADER_SIZE+(int)pow(2,ceil(log((double)pTable.pData.mem)/log(2))));
    mem_file=fopen("mem_log.txt","a+");
    fprintf(mem_file,"At time %d allocated %d bytes for process %d from %ld to %ld\n",getClk(),pTable.pData.mem,pTable.pData.id,((uint8_t *)pTable.memaddress-bptr)-HEADER_SIZE,((uint8_t *)pTable.memaddress-bptr)-HEADER_SIZE+(int)pow(2,ceil(log((double)pTable.pData.mem)/log(2))));
    fclose(mem_file);
    }
printf("At time %d allocated %d bytes for process %d from %ld to %ld\n",getClk(),pTable.pData.mem,pTable.pData.id,((uint8_t *)pTable.memaddress-bptr)-HEADER_SIZE,((uint8_t *)pTable.memaddress-bptr)-HEADER_SIZE+(int)pow(2,ceil(log((double)pTable.pData.mem)/log(2))));

push(&pTables,pTable,pTable.remainingTime);
}
}
//end new

    kill(pid,SIGSTOP);

    (*shmaddr_ps)--;
}
}



if(whichAlgorithm==RR)
{
//RoundRobin


if(i==1 && !isEmptyRR())
{dequeue(&inTurnProcess);
inTurnProcess.running=true;
i++;
kill(inTurnProcess.pid,SIGCONT);
 RT = (int *) shmat(inTurnProcess.shmID , (void *)0, 0);
 pOFile=fopen("RR_out.txt","a+");
     if (pOFile == NULL)
    {
        printf("Could not open output file \n");
    }
 printf("Process of Pid %d is to be scheduled \n",inTurnProcess.pid);
 WT=getClk()-inTurnProcess.pData.arrivaltime-inTurnProcess.pData.runningtime+inTurnProcess.remainingTime;
 fprintf(pOFile,"At Time %d Process %d started arr %d total %d remaining %d wait %d \n",curr_time,inTurnProcess.pData.id,inTurnProcess.pData.arrivaltime,inTurnProcess.pData.runningtime,inTurnProcess.remainingTime,WT);
 fclose(pOFile);
}


if( (!isEmptyRR()) || (inTurnProcess.finished==false) )
{
if((howLongRunning==quanta && curr_time==prev_time+1) || (inTurnProcess.finished==true ) )
{
howLongRunning=0;
inTurnProcess.running=false;
kill(inTurnProcess.pid,SIGSTOP);
WT=getClk()-inTurnProcess.pData.arrivaltime-inTurnProcess.pData.runningtime+inTurnProcess.remainingTime;
pOFile=fopen("RR_out.txt","a+");
     if (pOFile == NULL)
    {
        printf("Could not open output file \n");
    }
 fprintf(pOFile,"At Time %d Process %d stopped arr %d total %d remaining %d wait %d \n",curr_time,inTurnProcess.pData.id,inTurnProcess.pData.arrivaltime,inTurnProcess.pData.runningtime,inTurnProcess.remainingTime,WT);
fclose(pOFile);
if(!inTurnProcess.finished)
{
enqueue(inTurnProcess);
}
dequeue(&inTurnProcess);
kill(inTurnProcess.pid,SIGCONT);
WT=getClk()-inTurnProcess.pData.arrivaltime-inTurnProcess.pData.runningtime+inTurnProcess.remainingTime;
pOFile=fopen("RR_out.txt","a+");
     if (pOFile == NULL)
    {
        printf("Could not open output file \n");
    }
 fprintf(pOFile,"At Time %d Process %d resumed arr %d total %d remaining %d wait %d \n",curr_time,inTurnProcess.pData.id,inTurnProcess.pData.arrivaltime,inTurnProcess.pData.runningtime,inTurnProcess.remainingTime,WT);
fclose(pOFile);
inTurnProcess.running=true;
RT = (int *) shmat(inTurnProcess.shmID , (void *)0, 0);
 printf("Process of Pid %d is to be scheduled \n",inTurnProcess.pid);
}
else if(howLongRunning<quanta && inTurnProcess.running==true && curr_time==prev_time+1 )
{
kill(inTurnProcess.pid,SIGCONT);
(*RT)--;
inTurnProcess.remainingTime--;
howLongRunning++;
printf("Process %d is running with remaining time %d \n",inTurnProcess.pid, *RT);
if(*RT==0)
{
inTurnProcess.running=false;
inTurnProcess.finished=true;
WT=getClk()-inTurnProcess.pData.arrivaltime-inTurnProcess.pData.runningtime+inTurnProcess.remainingTime;
TA=getClk()-inTurnProcess.pData.arrivaltime;
WTA=(float)(getClk()-inTurnProcess.pData.arrivaltime)/(float)inTurnProcess.pData.arrivaltime;
pOFile=fopen("RR_out.txt","a+");
     if (pOFile == NULL)
    {
        printf("Could not open output file \n");
    }
 fprintf(pOFile,"At Time %d Process %d finished arr %d total %d remaining %d wait %d TA %d WTA %f \n",curr_time,inTurnProcess.pData.id,inTurnProcess.pData.arrivaltime,inTurnProcess.pData.runningtime,inTurnProcess.remainingTime,WT,TA,WTA);
fclose(pOFile);
printf("Process %d finished \n ",inTurnProcess.pid);

tot_wait+=WT;
tot_wta+=WTA;
tot_runtime+=inTurnProcess.pData.runningtime;
wta_arr[g]=WTA;
g+=1;

printf("At time %d freed %d bytes for process %d from %ld to %ld\n",getClk(),inTurnProcess.pData.mem,inTurnProcess.pData.id,((uint8_t *)inTurnProcess.memaddress-bptr)-HEADER_SIZE,((uint8_t *)inTurnProcess.memaddress-bptr)-HEADER_SIZE+(int)pow(2,ceil(log((double)inTurnProcess.pData.mem)/log(2))));
    mem_file=fopen("mem_log.txt","a+");
fprintf(mem_file,"At time %d freed %d bytes for process %d from %ld to %ld\n",getClk(),inTurnProcess.pData.mem,inTurnProcess.pData.id,((uint8_t *)inTurnProcess.memaddress-bptr)-HEADER_SIZE,((uint8_t *)inTurnProcess.memaddress-bptr)-HEADER_SIZE+(int)pow(2,ceil(log((double)inTurnProcess.pData.mem)/log(2))));
    fclose(mem_file);
free_mem(inTurnProcess.memaddress);


fin=fin+1;
if(len==fin) break;
}

}

}
else if(isEmptyRR()&& inTurnProcess.finished==true && curr_time==prev_time+1)
{
printf("Nothing To Schedule for the moment \n ");
}
}



else if(whichAlgorithm==HPF)
{
//HPF
int WT,TA;
float WTA;
if (isEmpty(&pTables)==1 && prev_time==curr_time-1 && processTurn.running==true)
{
decRT(&processTurn);
//printf("0-process %d is running with remaining time %d at time =%d \n",processTurn.pid, processTurn.remainingTime, getClk());
}
if(isEmpty(&pTables)==1 && prev_time==curr_time-1 && processTurn.running==true && processTurn.remainingTime==0)
{
processTurn.running=false;
WT=getClk()-processTurn.pData.arrivaltime-processTurn.pData.runningtime+processTurn.remainingTime;
TA=getClk()-processTurn.pData.arrivaltime;
WTA=(float)(getClk()-processTurn.pData.arrivaltime)/(float)processTurn.pData.runningtime;
    HPF_file=fopen("HPF_out.txt","a+");
fprintf(HPF_file,"At Time %d Process %d finished arr %d total %d remaining %d wait %d TA %d WTA %f \n",curr_time,processTurn.pData.id,processTurn.pData.arrivaltime,processTurn.pData.runningtime,processTurn.remainingTime,WT,TA,WTA);
    fclose(HPF_file);
printf("1-process %d finished with remaining time %d at time =%d \n",processTurn.pid, processTurn.remainingTime, getClk());

tot_wait+=WT;
tot_wta+=WTA;
tot_runtime+=processTurn.pData.runningtime;
wta_arr[g]=WTA;
g+=1;

printf("At time %d freed %d bytes for process %d from %ld to %ld\n",getClk(),processTurn.pData.mem,processTurn.pData.id,((uint8_t *)processTurn.memaddress-bptr)-HEADER_SIZE,((uint8_t *)processTurn.memaddress-bptr)-HEADER_SIZE+(int)pow(2,ceil(log((double)processTurn.pData.mem)/log(2))));
mem_file=fopen("mem_log.txt","a+");
        fprintf(mem_file,"At time %d freed %d bytes for process %d from %ld to %ld\n",getClk(),processTurn.pData.mem,processTurn.pData.id,((uint8_t *)processTurn.memaddress-bptr)-HEADER_SIZE,((uint8_t *)processTurn.memaddress-bptr)-HEADER_SIZE+(int)pow(2,ceil(log((double)processTurn.pData.mem)/log(2))));
    fclose(mem_file);
free_mem(processTurn.memaddress);

fin=fin+1;
if(len==fin) break;
}
if(isEmpty(&pTables)==0)
{
i=0;
    if(pTables->next==NULL&& processTurn.running!=true )
    {
    pop(&pTables, &processTurn);
    processTurn.running=true;
        HPF_file=fopen("HPF_out.txt","a+");
    fprintf(HPF_file,"At Time %d Process %d started arr %d total %d remaining %d wait %d \n",getClk(),processTurn.pData.id,processTurn.pData.arrivaltime,processTurn.pData.runningtime,processTurn.remainingTime,(getClk()-processTurn.pData.arrivaltime)-(processTurn.pData.runningtime-processTurn.remainingTime));
        fclose(HPF_file);
    printf("2-Process %d started with remaining time =%d at time =%d \n",processTurn.pid,processTurn.remainingTime,getClk());
    z++;
    i++;
    }

    if(prev_time==curr_time-1 && i==0)
    {
        if(processTurn.running==true && processTurn.remainingTime==0)
        {
       // pop(&pTables,&processTurn);
        processTurn.running=false;
        WT=getClk()-processTurn.pData.arrivaltime-processTurn.pData.runningtime+processTurn.remainingTime;
        TA=getClk()-processTurn.pData.arrivaltime;
        WTA=(float)(getClk()-processTurn.pData.arrivaltime)/(float)processTurn.pData.runningtime;
            HPF_file=fopen("HPF_out.txt","a+");
        fprintf(HPF_file,"At Time %d Process %d finished arr %d total %d remaining %d wait %d TA %d WTA %f \n",curr_time,processTurn.pData.id,processTurn.pData.arrivaltime,processTurn.pData.runningtime,processTurn.remainingTime,WT,TA,WTA);
            fclose(HPF_file);
        printf("3-process %d finished with remaining time =%d at time=%d\n", processTurn.pid, processTurn.remainingTime, getClk());

        tot_wait+=WT;
        tot_wta+=WTA;
        tot_runtime+=processTurn.pData.runningtime;
        wta_arr[g]=WTA;
        g+=1;

printf("At time %d freed %d bytes for process %d from %ld to %ld\n",getClk(),processTurn.pData.mem,processTurn.pData.id,((uint8_t *)processTurn.memaddress-bptr)-HEADER_SIZE,((uint8_t *)processTurn.memaddress-bptr)-HEADER_SIZE+(int)pow(2,ceil(log((double)processTurn.pData.mem)/log(2))));
mem_file=fopen("mem_log.txt","a+");
        fprintf(mem_file,"At time %d freed %d bytes for process %d from %ld to %ld\n",getClk(),processTurn.pData.mem,processTurn.pData.id,((uint8_t *)processTurn.memaddress-bptr)-HEADER_SIZE,((uint8_t *)processTurn.memaddress-bptr)-HEADER_SIZE+(int)pow(2,ceil(log((double)processTurn.pData.mem)/log(2))));
    fclose(mem_file);
 free_mem(processTurn.memaddress);

        fin=fin+1;
if(len==fin) break;

        pop(&pTables, &processTurn);
        processTurn.running=true;

        if(processTurn.remainingTime<processTurn.pData.runningtime)
        {
            HPF_file=fopen("HPF_out.txt","a+");
        fprintf(HPF_file,"At Time %d Process %d resumed arr %d total %d remaining %d wait %d \n",getClk(),processTurn.pData.id,processTurn.pData.arrivaltime,processTurn.pData.runningtime,processTurn.remainingTime,(getClk()-processTurn.pData.arrivaltime)-(processTurn.pData.runningtime-processTurn.remainingTime));
        printf("4-process %d resumed with remaining time= %d at time=%d\n", processTurn.pid, processTurn.remainingTime,getClk());
        }
        else
        {
            HPF_file=fopen("HPF_out.txt","a+");
        fprintf(HPF_file,"At Time %d Process %d started arr %d total %d remaining %d wait %d \n",getClk(),processTurn.pData.id,processTurn.pData.arrivaltime,processTurn.pData.runningtime,processTurn.remainingTime,(getClk()-processTurn.pData.arrivaltime)-(processTurn.pData.runningtime-processTurn.remainingTime));
            fclose(HPF_file);
        printf("4-process %d started with remaining time= %d at time=%d\n", processTurn.pid, processTurn.remainingTime,getClk());
        }
        }
        else if( pTables!=NULL &&(peek(&pTables).pData.priority) < (processTurn.pData.priority)&& processTurn.running==true)
        {
        decRT(&processTurn);
        processTurn.running=false;
            HPF_file=fopen("HPF_out.txt","a+");
        fprintf(HPF_file,"At Time %d Process %d stopped arr %d total %d remaining %d wait %d \n",getClk(),processTurn.pData.id,processTurn.pData.arrivaltime,processTurn.pData.runningtime,processTurn.remainingTime,(getClk()-processTurn.pData.arrivaltime)-(processTurn.pData.runningtime-processTurn.remainingTime));
            fclose(HPF_file);
        printf("5-process %d stopped with remaining time= %d at time=%d\n", processTurn.pid, processTurn.remainingTime,getClk());
        push(&pTables,processTurn,processTurn.pData.priority);
        pop(&pTables,&processTurn);
        processTurn.running=true;
        if(processTurn.remainingTime<processTurn.pData.runningtime)
        {
            HPF_file=fopen("HPF_out.txt","a+");
        fprintf(HPF_file,"At Time %d Process %d resumed arr %d total %d remaining %d wait %d \n",getClk(),processTurn.pData.id,processTurn.pData.arrivaltime,processTurn.pData.runningtime,processTurn.remainingTime,(getClk()-processTurn.pData.arrivaltime)-(processTurn.pData.runningtime-processTurn.remainingTime));
            fclose(HPF_file);
        printf("6-process %d resumed with remaining time= %d at time=%d\n", processTurn.pid, processTurn.remainingTime,getClk());
        }
        else
        {
            HPF_file=fopen("HPF_out.txt","a+");
        fprintf(HPF_file,"At Time %d Process %d started arr %d total %d remaining %d wait %d \n",getClk(),processTurn.pData.id,processTurn.pData.arrivaltime,processTurn.pData.runningtime,processTurn.remainingTime,(getClk()-processTurn.pData.arrivaltime)-(processTurn.pData.runningtime-processTurn.remainingTime));
            fclose(HPF_file);
        printf("6-process %d started with remaining time= %d at time=%d\n", processTurn.pid, processTurn.remainingTime,getClk());
        }
        }
        else
        {
        processTurn.running=true;
        decRT(&processTurn);
        if(processTurn.remainingTime==0)
        {
        processTurn.running=false;
        WT=getClk()-processTurn.pData.arrivaltime-processTurn.pData.runningtime+processTurn.remainingTime;
        TA=getClk()-processTurn.pData.arrivaltime;
        WTA=(float)(getClk()-processTurn.pData.arrivaltime)/(float)processTurn.pData.runningtime;
            HPF_file=fopen("HPF_out.txt","a+");
        fprintf(HPF_file,"At Time %d Process %d finished arr %d total %d remaining %d wait %d TA %d WTA %f \n",curr_time,processTurn.pData.id,processTurn.pData.arrivaltime,processTurn.pData.runningtime,processTurn.remainingTime,WT,TA,WTA);
            fclose(HPF_file);
        printf("7-process %d finished with remaining time =%d at time=%d\n", processTurn.pid, processTurn.remainingTime, getClk());

        tot_wait+=WT;
        tot_wta+=WTA;
        tot_runtime+=processTurn.pData.runningtime;
        wta_arr[g]=WTA;
        g+=1;
printf("At time %d freed %d bytes for process %d from %ld to %ld\n",getClk(),processTurn.pData.mem,processTurn.pData.id,((uint8_t *)processTurn.memaddress-bptr)-HEADER_SIZE,((uint8_t *)processTurn.memaddress-bptr)-HEADER_SIZE+(int)pow(2,ceil(log((double)processTurn.pData.mem)/log(2))));
mem_file=fopen("mem_log.txt","a+");
        fprintf(mem_file,"At time %d freed %d bytes for process %d from %ld to %ld\n",getClk(),processTurn.pData.mem,processTurn.pData.id,((uint8_t *)processTurn.memaddress-bptr)-HEADER_SIZE,((uint8_t *)processTurn.memaddress-bptr)-HEADER_SIZE+(int)pow(2,ceil(log((double)processTurn.pData.mem)/log(2))));
    fclose(mem_file);
free_mem(processTurn.memaddress);
        fin=fin+1;
if(len==fin) break;

        pop(&pTables,&processTurn);
        processTurn.running=true;

        if(processTurn.remainingTime<processTurn.pData.runningtime)
        {
            HPF_file=fopen("HPF_out.txt","a+");
        fprintf(HPF_file,"At Time %d Process %d resumed arr %d total %d remaining %d wait %d \n",getClk(),processTurn.pData.id,processTurn.pData.arrivaltime,processTurn.pData.runningtime,processTurn.remainingTime,(getClk()-processTurn.pData.arrivaltime)-(processTurn.pData.runningtime-processTurn.remainingTime));
            fclose(HPF_file);
        printf("8-process %d resumed with remaining time= %d at time=%d\n", processTurn.pid, processTurn.remainingTime,getClk());
        }
        else
        {
            HPF_file=fopen("HPF_out.txt","a+");
        fprintf(HPF_file,"At Time %d Process %d started arr %d total %d remaining %d wait %d \n",getClk(),processTurn.pData.id,processTurn.pData.arrivaltime,processTurn.pData.runningtime,processTurn.remainingTime,(getClk()-processTurn.pData.arrivaltime)-(processTurn.pData.runningtime-processTurn.remainingTime));
            fclose(HPF_file);
        printf("8-process %d started with remaining time= %d at time=%d\n", processTurn.pid, processTurn.remainingTime,getClk());
        }
        }
        }
    }

}
}
// new
else if(whichAlgorithm==SJF)
{
//SJF
int WT,TA;
float WTA;
if (isEmpty(&pTables)==1 && prev_time==curr_time-1 && processTurn.running==true)
{
decRT(&processTurn);
//printf("0-process %d is running with remaining time %d at time =%d \n",processTurn.pid, processTurn.remainingTime, getClk());
}
if(isEmpty(&pTables)==1 && prev_time==curr_time-1 && processTurn.running==true && processTurn.remainingTime==0)
{
processTurn.running=false;
WT=getClk()-processTurn.pData.arrivaltime-processTurn.pData.runningtime+processTurn.remainingTime;
TA=getClk()-processTurn.pData.arrivaltime;
WTA=(float)(getClk()-processTurn.pData.arrivaltime)/(float)processTurn.pData.runningtime;
    SJF_file=fopen("SJF_out.txt","a+");
fprintf(SJF_file,"At Time %d Process %d finished arr %d total %d remaining %d wait %d TA %d WTA %f \n",curr_time,processTurn.pData.id,processTurn.pData.arrivaltime,processTurn.pData.runningtime,processTurn.remainingTime,WT,TA,WTA);
    fclose(SJF_file);
printf("1-process %d finished with remaining time %d at time =%d \n",processTurn.pid, processTurn.remainingTime, getClk());

        tot_wait+=WT;
        tot_wta+=WTA;
        tot_runtime+=processTurn.pData.runningtime;
        wta_arr[g]=WTA;
        g+=1;

printf("At time %d freed %d bytes for process %d from %ld to %ld\n",getClk(),processTurn.pData.mem,processTurn.pData.id,((uint8_t *)processTurn.memaddress-bptr)-HEADER_SIZE,((uint8_t *)processTurn.memaddress-bptr)-HEADER_SIZE+(int)pow(2,ceil(log((double)processTurn.pData.mem)/log(2))));
mem_file=fopen("mem_log.txt","a+");
        fprintf(mem_file,"At time %d freed %d bytes for process %d from %ld to %ld\n",getClk(),processTurn.pData.mem,processTurn.pData.id,((uint8_t *)processTurn.memaddress-bptr)-HEADER_SIZE,((uint8_t *)processTurn.memaddress-bptr)-HEADER_SIZE+(int)pow(2,ceil(log((double)processTurn.pData.mem)/log(2))));
    fclose(mem_file);
 free_mem(processTurn.memaddress);

fin=fin+1;
if(len==fin) break;
}
if(isEmpty(&pTables)==0)
{
i=0;
    if(pTables->next==NULL&& processTurn.running!=true )
    {
    pop(&pTables, &processTurn);
    processTurn.running=true;
        SJF_file=fopen("SJF_out.txt","a+");
    fprintf(SJF_file,"At Time %d Process %d started arr %d total %d remaining %d wait %d \n",getClk(),processTurn.pData.id,processTurn.pData.arrivaltime,processTurn.pData.runningtime,processTurn.remainingTime,(getClk()-processTurn.pData.arrivaltime)-(processTurn.pData.runningtime-processTurn.remainingTime));
        fclose(SJF_file);
    printf("2-Process %d started with remaining time =%d at time =%d \n",processTurn.pid,processTurn.remainingTime,getClk());
    z++;
    i++;
    }

    if(prev_time==curr_time-1 && i==0)
    {
        if(processTurn.running==true && processTurn.remainingTime==0)
        {
       // pop(&pTables,&processTurn);
        processTurn.running=false;
        WT=getClk()-processTurn.pData.arrivaltime-processTurn.pData.runningtime+processTurn.remainingTime;
        TA=getClk()-processTurn.pData.arrivaltime;
        WTA=(float)(getClk()-processTurn.pData.arrivaltime)/(float)processTurn.pData.runningtime;
            SJF_file=fopen("SJF_out.txt","a+");
        fprintf(SJF_file,"At Time %d Process %d finished arr %d total %d remaining %d wait %d TA %d WTA %f \n",curr_time,processTurn.pData.id,processTurn.pData.arrivaltime,processTurn.pData.runningtime,processTurn.remainingTime,WT,TA,WTA);
            fclose(SJF_file);
        printf("3-process %d finished with remaining time =%d at time=%d\n", processTurn.pid, processTurn.remainingTime, getClk());

        tot_wait+=WT;
        tot_wta+=WTA;
        tot_runtime+=processTurn.pData.runningtime;
        wta_arr[g]=WTA;
        g+=1;

printf("At time %d freed %d bytes for process %d from %ld to %ld\n",getClk(),processTurn.pData.mem,processTurn.pData.id,((uint8_t *)processTurn.memaddress-bptr)-HEADER_SIZE,((uint8_t *)processTurn.memaddress-bptr)-HEADER_SIZE+(int)pow(2,ceil(log((double)processTurn.pData.mem)/log(2))));
 mem_file=fopen("mem_log.txt","a+");
        fprintf(mem_file,"At time %d freed %d bytes for process %d from %ld to %ld\n",getClk(),processTurn.pData.mem,processTurn.pData.id,((uint8_t *)processTurn.memaddress-bptr)-HEADER_SIZE,((uint8_t *)processTurn.memaddress-bptr)-HEADER_SIZE+(int)pow(2,ceil(log((double)processTurn.pData.mem)/log(2))));
    fclose(mem_file);
 free_mem(processTurn.memaddress);

        fin=fin+1;
if(len==fin) break;

        pop(&pTables, &processTurn);
        processTurn.running=true;

        if(processTurn.remainingTime<processTurn.pData.runningtime)
        {
            SJF_file=fopen("SJF_out.txt","a+");
        fprintf(SJF_file,"At Time %d Process %d resumed arr %d total %d remaining %d wait %d \n",getClk(),processTurn.pData.id,processTurn.pData.arrivaltime,processTurn.pData.runningtime,processTurn.remainingTime,(getClk()-processTurn.pData.arrivaltime)-(processTurn.pData.runningtime-processTurn.remainingTime));
        printf("4-process %d resumed with remaining time= %d at time=%d\n", processTurn.pid, processTurn.remainingTime,getClk());
        }
        else
        {
            SJF_file=fopen("SJF_out.txt","a+");
        fprintf(SJF_file,"At Time %d Process %d started arr %d total %d remaining %d wait %d \n",getClk(),processTurn.pData.id,processTurn.pData.arrivaltime,processTurn.pData.runningtime,processTurn.remainingTime,(getClk()-processTurn.pData.arrivaltime)-(processTurn.pData.runningtime-processTurn.remainingTime));
            fclose(SJF_file);
        printf("4-process %d started with remaining time= %d at time=%d\n", processTurn.pid, processTurn.remainingTime,getClk());
        }
        }
        else if( pTables!=NULL &&(peek(&pTables).remainingTime) < (processTurn.remainingTime)&& processTurn.running==true)
        {
        decRT(&processTurn);
        processTurn.running=false;
            SJF_file=fopen("SJF_out.txt","a+");
        fprintf(SJF_file,"At Time %d Process %d stopped arr %d total %d remaining %d wait %d \n",getClk(),processTurn.pData.id,processTurn.pData.arrivaltime,processTurn.pData.runningtime,processTurn.remainingTime,(getClk()-processTurn.pData.arrivaltime)-(processTurn.pData.runningtime-processTurn.remainingTime));
            fclose(SJF_file);
        printf("5-process %d stopped with remaining time= %d at time=%d\n", processTurn.pid, processTurn.remainingTime,getClk());
        push(&pTables,processTurn,processTurn.pData.priority);
        pop(&pTables,&processTurn);
        processTurn.running=true;
        if(processTurn.remainingTime<processTurn.pData.runningtime)
        {
            SJF_file=fopen("SJF_out.txt","a+");
        fprintf(SJF_file,"At Time %d Process %d resumed arr %d total %d remaining %d wait %d \n",getClk(),processTurn.pData.id,processTurn.pData.arrivaltime,processTurn.pData.runningtime,processTurn.remainingTime,(getClk()-processTurn.pData.arrivaltime)-(processTurn.pData.runningtime-processTurn.remainingTime));
            fclose(SJF_file);
        printf("6-process %d resumed with remaining time= %d at time=%d\n", processTurn.pid, processTurn.remainingTime,getClk());
        }
        else
        {
            SJF_file=fopen("SJF_out.txt","a+");
        fprintf(SJF_file,"At Time %d Process %d started arr %d total %d remaining %d wait %d \n",getClk(),processTurn.pData.id,processTurn.pData.arrivaltime,processTurn.pData.runningtime,processTurn.remainingTime,(getClk()-processTurn.pData.arrivaltime)-(processTurn.pData.runningtime-processTurn.remainingTime));
            fclose(SJF_file);
        printf("6-process %d started with remaining time= %d at time=%d\n", processTurn.pid, processTurn.remainingTime,getClk());
        }
        }
        else
        {
        processTurn.running=true;
        decRT(&processTurn);
        if(processTurn.remainingTime==0)
        {
        processTurn.running=false;
        WT=getClk()-processTurn.pData.arrivaltime-processTurn.pData.runningtime+processTurn.remainingTime;
        TA=getClk()-processTurn.pData.arrivaltime;
        WTA=(float)(getClk()-processTurn.pData.arrivaltime)/(float)processTurn.pData.runningtime;
            SJF_file=fopen("SJF_out.txt","a+");
        fprintf(SJF_file,"At Time %d Process %d finished arr %d total %d remaining %d wait %d TA %d WTA %f \n",curr_time,processTurn.pData.id,processTurn.pData.arrivaltime,processTurn.pData.runningtime,processTurn.remainingTime,WT,TA,WTA);
            fclose(SJF_file);
        printf("7-process %d finished with remaining time =%d at time=%d\n", processTurn.pid, processTurn.remainingTime, getClk());

        tot_wait+=WT;
        tot_wta+=WTA;
        tot_runtime+=processTurn.pData.runningtime;
        wta_arr[g]=WTA;
        g+=1;

        printf("At time %d freed %d bytes for process %d from %ld to %ld\n",getClk(),processTurn.pData.mem,processTurn.pData.id,((uint8_t *)processTurn.memaddress-bptr)-HEADER_SIZE,((uint8_t *)processTurn.memaddress-bptr)-HEADER_SIZE+(int)pow(2,ceil(log((double)processTurn.pData.mem)/log(2))));
    mem_file=fopen("mem_log.txt","a+");
        fprintf(mem_file,"At time %d freed %d bytes for process %d from %ld to %ld\n",getClk(),processTurn.pData.mem,processTurn.pData.id,((uint8_t *)processTurn.memaddress-bptr)-HEADER_SIZE,((uint8_t *)processTurn.memaddress-bptr)-HEADER_SIZE+(int)pow(2,ceil(log((double)processTurn.pData.mem)/log(2))));
    fclose(mem_file);
 free_mem(processTurn.memaddress);

        fin=fin+1;
if(len==fin) break;

        pop(&pTables,&processTurn);
        processTurn.running=true;

        if(processTurn.remainingTime<processTurn.pData.runningtime)
        {
            SJF_file=fopen("SJF_out.txt","a+");
        fprintf(SJF_file,"At Time %d Process %d resumed arr %d total %d remaining %d wait %d \n",getClk(),processTurn.pData.id,processTurn.pData.arrivaltime,processTurn.pData.runningtime,processTurn.remainingTime,(getClk()-processTurn.pData.arrivaltime)-(processTurn.pData.runningtime-processTurn.remainingTime));
            fclose(SJF_file);
        printf("8-process %d resumed with remaining time= %d at time=%d\n", processTurn.pid, processTurn.remainingTime,getClk());
        }
        else
        {
            SJF_file=fopen("SJF_out.txt","a+");
        fprintf(SJF_file,"At Time %d Process %d started arr %d total %d remaining %d wait %d \n",getClk(),processTurn.pData.id,processTurn.pData.arrivaltime,processTurn.pData.runningtime,processTurn.remainingTime,(getClk()-processTurn.pData.arrivaltime)-(processTurn.pData.runningtime-processTurn.remainingTime));
            fclose(SJF_file);
        printf("8-process %d started with remaining time= %d at time=%d\n", processTurn.pid, processTurn.remainingTime,getClk());
        }
        }
        }
    }

}
}
// end new










prev_time=curr_time;
}
float cpu_utl=(float)tot_runtime/getClk();
float avg_wta=(float)tot_wta/len;
float avg_wait=(float)tot_wait/len;
float std_wta=calc_SD(wta_arr,len);
perf_file=fopen("perf_out.txt","a+");
fprintf(perf_file,"CPU Utlization = %f \n", cpu_utl);
fprintf(perf_file,"Avg WTA =%f \n", avg_wta);
fprintf(perf_file,"Avg Waiting =%f \n", avg_wait);
fprintf(perf_file,"Std WTA =%f \n", std_wta);

exit(getClk());
}




    //TODO implement the scheduler :)
    //upon termination release the clock resources

    //destroyClk(false);

