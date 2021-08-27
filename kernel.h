
/*
Returns the size of the queue
*/
int size();

/*
Pops the pcb at the head of the linked list.
pop will cause an error if linkedlist is empty.
Always check size of queue using size()
*/
struct PCB* pop();

/*
Adds a pcb to the tail of the linked list
*/
void addToReady(struct PCB *pcb);


int scheduler();

/*
Flushes every pcb off the ready queue in the case of a load error
*/
void emptyReadyQueue();

/*
Given a frameNumber, it returns the PCB that was allocated
this frame.
*/
struct PCB* getFrameOwner(int frameNumber);