#include <stdio.h>      //if you don't use scanf/printf change this include
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <math.h>
#include <memory.h>
#include <stdint.h>
typedef short bool;
#define true 1
#define false 0

#define SHKEY 300
#define max 100

#define HEADER_SIZE 8
#define MIN_ALLOC_POW 4
#define MIN_ALLOC ((size_t)1 << MIN_ALLOC_POW);
#define MAX_ALLOC_POW 10
#define MAX_ALLOC ((size_t)1 << MAX_ALLOC_POW)
#define CONT_COUNT (MAX_ALLOC_POW - MIN_ALLOC_POW + 1)
#define SHKEY 300
#define max 100
typedef struct list_mem {
  struct list_mem *prev, *next;
} list_mem;
static uint8_t isNodeSplit[(1 << (CONT_COUNT - 1)) / 8];
static list_mem containers[CONT_COUNT];
static uint8_t *bptr;/*pointer of mkin used tree value*/
static uint8_t *mxptr; /*pointer of max used tree value*/
static size_t container_limit; /*tree size tracker value*/

///==============================
//don't mess with this variable//
int * shmaddr;                 //
//===============================

int getRT(int pid)
{
int shmid_sp = shmget(pid, 8, 0777);
int *shmaddr_sp = (int *) shmat(shmid_sp, (void *)0, 0);
return *shmaddr_sp;
}

static void makelist(list_mem *list) {
  list->next = list;
  list->prev = list;

}

static void listPush(list_mem *list, list_mem *input) {
  list_mem *prev = list->prev;
  input->next = list;
  input->prev = prev;
  list->prev = input;
  prev->next = input;

}
static void listRemove(list_mem* list) {
  list_mem *prev = list->prev;
  list_mem *next = list->next;
  prev->next = next;
  next->prev = prev;
}
static list_mem *listPop(list_mem*list) {
  list_mem *back = list->prev;
  if (back == list) return NULL;
  listRemove(back);
  return back;
}

static int mxptrNewVal(uint8_t *new_value) {
  if (new_value > mxptr) {
    if (brk(new_value)) return 0;

    mxptr = new_value;
  }
  return 1;
}

//Maps node index to memoy address node represents
static uint8_t *MapN2A(size_t index, size_t container) {
  return bptr + ((index - (1 << container) + 1) << (MAX_ALLOC_POW - container));
}
//Maps memory address to node representing it
static size_t MapA2N(uint8_t *ptr, size_t container) {
  return ((ptr - bptr) >> (MAX_ALLOC_POW - container)) + (1 << container) - 1;
}

static int isParentSplit(size_t index) {
  index = (index - 1) / 2;
  return (isNodeSplit[index / 8] >> (index % 8)) & 1;
}

static void FisParentSplit(size_t index) {
  index = (index - 1) / 2;
  isNodeSplit[index / 8] ^= 1 << (index % 8);
}

static size_t requestContainer(size_t request) {
  size_t container = CONT_COUNT - 1;
  size_t size = MIN_ALLOC;

  while (size < request) {
    container--;
    size *= 2;
  }

  return container;
}
//lowercontainerlimit
static int GrowTree(size_t container) {
  while (container < container_limit) {
    size_t root = MapA2N(bptr, container_limit);
    uint8_t *right_child;

    if (!isParentSplit(root)) {
      listRemove((list_mem *)bptr);
      makelist(&containers[--container_limit]);
      listPush(&containers[container_limit], (list_mem *)bptr);
      continue;
    }
    right_child = MapN2A(root + 1, container_limit);
    if (!mxptrNewVal(right_child + sizeof(list_mem))) {
      return 0;
    }
    listPush(&containers[container_limit], (list_mem *)right_child);
    makelist(&containers[--container_limit]);
    root = (root - 1) / 2;
    if (root != 0) {
      FisParentSplit(root);
    }
  }

  return 1;
}

int getClk()
{
    return *shmaddr;
}


/*
 * All process call this function at the beginning to establish communication between them and the clock module.
 * Again, remember that the clock is only emulation!
*/
void initClk()
{
    int shmid = shmget(SHKEY, 4, 0444);
    while ((int)shmid == -1)
    {
        //Make sure that the clock exists
        printf("Wait! The clock not initialized yet!\n");
        sleep(1);
        shmid = shmget(SHKEY, 4, 0444);
    }
    shmaddr = (int *) shmat(shmid, (void *)0, 0);
}


/*
 * All process call this function at the end to release the communication
 * resources between them and the clock module.
 * Again, Remember that the clock is only emulation!
 * Input: terminateAll: a flag to indicate whether that this is the end of simulation.
 *                      It terminates the whole system and releases resources.
*/

float calc_SD(float wta_data[], int s) {
    int sum = 0;
    float mean;
    float SD = 0.0;
    int i;
    for (i = 0; i < s; ++i) {
        sum += wta_data[i];
    }
    mean = sum / s;
    for (i = 0; i < s; ++i)
        SD += pow(wta_data[i] - mean, 2);

        return sqrt(SD/s);

}



void destroyClk(bool terminateAll)
{
    shmdt(shmaddr);
    if (terminateAll)
    {
        killpg(getpgrp(), SIGINT);
    }
}
typedef struct
{
    int arrivaltime;
    int priority;
    int runningtime;
    int id;
    int mem;
}processData;

typedef struct{
processData pData;
int remainingTime;
int pid;
int finishTime;
bool running;
bool finished;
int shmID;
void* memaddress;
}processTable;

enum Algorithm
{
RR,
HPF,
SJF
};



typedef struct {
    int priority;
    processData *data;
} node_t;

typedef struct {
    node_t *nodes;
    int len;
    int size;
} heap_t;

void pushH (heap_t *h, int priority, processData *data) {
    if (h->len + 1 >= h->size) {
        h->size = h->size ? h->size * 2 : 4;
        h->nodes = (node_t *)realloc(h->nodes, h->size * sizeof (node_t));
    }
    int i = h->len + 1;
    int j = i / 2;
    while (i > 1 && h->nodes[j].priority > priority) {
        h->nodes[i] = h->nodes[j];
        i = j;
        j = j / 2;
    }
    h->nodes[i].priority = priority;
    (h->nodes[i].data) = data;
    h->len++;
}

processData * popH (heap_t *h) {
    int i, j, k;
    if (!h->len) {
        return NULL;
    }
    processData *data = h->nodes[1].data;

    h->nodes[1] = h->nodes[h->len];

    h->len--;

    i = 1;
    while (i!=h->len+1) {
        k = h->len+1;
        j = 2 * i;
        if (j <= h->len && h->nodes[j].priority < h->nodes[k].priority) {
            k = j;
        }
        if (j + 1 <= h->len && h->nodes[j + 1].priority < h->nodes[k].priority) {
            k = j + 1;
        }
        h->nodes[i] = h->nodes[k];
        i = k;
    }
    return data;
}



typedef struct node {
   processTable data;
   int priority;
   struct node* next;
} Node;
Node* newNode(processTable d, int p) {
   Node* temp = (Node*)malloc(sizeof(Node));
   temp->data = d;
   temp->priority = p;
   temp->next = NULL;
   return temp;
}
int isEmpty(Node** head) {
   return (*head) == NULL;
}
processTable peek(Node** head) {
   return (*head)->data;
}
void pop(Node** head, processTable *t) {
  Node* temp = *head;
   *t=(*head)->data;
   (*head) = (*head)->next;
   free(temp);

}
void push(Node** head, processTable d, int p) {
   Node* start = (*head);
   Node* temp = newNode(d, p);
   if ((*head)->priority > p) {
      temp->next = *head;
      (*head) = temp;
   } else {
      while (start->next != NULL &&
      start->next->priority < p) {
         start = start->next;
      }
      // Either at the ends of the list
      // or at required position
      temp->next = start->next;
      start->next = temp;
   }
}

struct NodeRR
{
	processTable data;
	struct NodeRR* next;
};
struct NodeRR *f = NULL;
struct NodeRR *r = NULL;
void enqueue(processTable d) //Insert elements in Queue
{
	struct NodeRR* n;
	n = (struct NodeRR*)malloc(sizeof(struct NodeRR));
	n->data = d;
	n->next = NULL;
	if((r==NULL)&&(f==NULL))
	{
		f = r = n;
		r->next = f;
	}
	else
	{
		r->next = n;
		r = n;
		n->next = f;
	}
}
bool dequeue(processTable* tble)
{
	struct NodeRR* t;
	t = f;
	if((f==NULL)&&(r==NULL))
	{
		//printf("\nQueue is Empty");
		return 0;
    }
	else if(f == r){
	*tble=f->data;
		f = r = NULL;
		free(t);
		return 1;
	}
	else{
	*tble=f->data;
		f = f->next;
		r->next = f;
		free(t);
		return 1;
	}

}

void print(){ // Print the elements of Queue
	struct NodeRR* t;
	t = f;
	if((f==NULL)&&(r==NULL))
		printf("\nQueue is Empty");
	else{
		do{
			printf("\n%d",t->data.pid);
			t = t->next;
		}while(t != f);
	}
}
bool isEmptyRR()
{
return f==NULL;
}
processTable peekRR()
{

return f->data;
}
