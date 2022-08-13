#include "headers.h"

/* Modify this file as needed*/


int main(int agrc, char * argv[])
{

    int remainingtime=getRT(getpid());

    printf("Process of pid %d forked with remaining time=%d\n",getpid(),remainingtime);
    initClk();

    //TODO it needs to get the remaining time from somewhere
    //remainingtime = ??;
    while (getRT(getpid()) > 0)
    {
        remainingtime = getRT(getpid());
        printf("I'm process %d my remaining time is %d \n", getpid(),remainingtime);
        sleep(1);
    }
printf("I , Process %d  have Finished \n",getpid());
    //destroyClk(false);

    return 0;
}
