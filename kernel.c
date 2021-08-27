#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"shell.h"
#include"pcb.h"
#include"ram.h"
#include"cpu.h"
#include"memorymanager.h"
#include"interpreter.h"
#include <dirent.h>
#include <errno.h>
#include "DISK_driver.h"




/*
This is a node in the Ready Queue implemented as 
a linked list.
A node holds a PCB pointer and a pointer to the next node.
PCB: PCB
next: next node
*/
typedef struct ReadyQueueNode {
    PCB*  PCB;
    struct ReadyQueueNode* next;
} ReadyQueueNode;

ReadyQueueNode* head = NULL;
ReadyQueueNode* tail = NULL;
int sizeOfQueue = 0;

/*
This function will perform the OS boot sequence.
It will clear the RAM, i.e set every cell to NULL
It will delete the content of the backing store directory named "BackingStore"
*/
int boot(){
    //Clears the RAM
    clearRAM(); 
    initIO();
    // Get a directory pointer
    DIR* dir = opendir("BackingStore");
    // if directory exists, close it and remove its content
    if (dir) {
        closedir(dir);
        system("rm -r BackingStore/*");
    //if directory does not exist, create it
    } else if (ENOENT == errno) {
        system("mkdir BackingStore");
    }
}

/*
This is the kernel function which delegates the flow of operation
to the shellUI function.
At the end, it also deletes the files in the BackingStore
*/
int kernel()
{
    // runs the shell
    int errorCode = shellUI();
    // DELETE FILES IN BACKING STORE
    //system("rm -r BackingStore/*");
    return errorCode;
}

/*
This is the main function.
*/
int main(int argc, char const *argv[])
{
    int error = 0;
    boot();
    error = kernel();
    return error;
}

/*
Adds a pcb to the tail of the linked list
*/
void addToReady(struct PCB* pcb) {
    ReadyQueueNode* newNode = (ReadyQueueNode *)malloc(sizeof(ReadyQueueNode));
    newNode->PCB = pcb;
    newNode->next = NULL;
    if (head == NULL){
        head = newNode;
        tail = newNode;
    } else {
        tail->next = newNode;
        tail = newNode;
    }
    sizeOfQueue++;
}

/*
Returns the size of the queue
*/
int size(){
    return sizeOfQueue;
}

/*
Pops the pcb at the head of the linked list.
pop will cause an error if linkedlist is empty.
Always check size of queue using size()
*/
struct PCB* pop(){
    PCB* topNode = head->PCB;
    ReadyQueueNode * temp = head;
    if (head == tail){
        head = NULL;
        tail = NULL;
    } else {
        head = head->next;
    }
    free(temp);
    sizeOfQueue--;
    return topNode;
}


/*
Current at a pageFault
The page fault operation follows these steps: 
1. Determine the next page: PC_page++. (our scripts never loop) 

2. If PC_page is beyond pages_max then the program terminates, 
otherwise we check to see if the frame for that page exists in the pageTable array. 

3. If pageTable[PC_page] is valid then we have the frame number 
and can do PC=ram[frame] and reset PC_offset to zero. 

4. If pageTable[PC_page] is NOT valid then we need 
to find the page on disk and update the PCB page table 
and possibly the victim’s page table. 
Start by (a) finding the page in the backing store, then 
(b) finding space in RAM (either find a free cell or select a victim), finally 
(c) update the page tables, 
(d) update RAM frame with instructions, and do 
(e) PC=ram[frame] and 
(f) reset PC_offset to zero. 

5. Since the PCB was interrupted, it has lost it quanta, 
even when there were some quanta left. 
Store everything back into the PCB. 
Place the PCB at the back of the Ready queue.
*/
int resolvePageFault(struct PCB* pcb){
    // Go to next page
    pcb->PC_page ++;
    //If no more pages, free the frames for that pcb and its pointer
    if (pcb->PC_page == pcb->pages_max){
        freeFramesForPCB(pcb);
        free(pcb);
    } else {
        // if the current page does not have a frame allocated to it
        if (pcb->pageTable[pcb->PC_page] == -1){

            //Finds the file in the backing store for that PCB
            char filename[50] = "BackingStore/file";
            char* extension = ".txt";
            char buffer[8];
            sprintf(buffer,"%d",pcb->PID);
            strcat(filename,buffer);
            strcat(filename,extension);
            FILE* fp = fopen(filename,"r");
            // LATER CATER FOR NULL FILE POINTER

            //looks for a frame in ram
            int frameNum = findFrame();
            
            int victimframe = -1;
            //if no free frame, choose a victim
            if (frameNum == -1) {
                frameNum = findVictim(pcb);
                victimframe = frameNum;
            }
            //loads the current page in the frame
            loadPage(pcb->PC_page,fp,frameNum);
            //update the page table for current pcb ( possible for victim as well if any)
            updatePageTable(pcb,pcb->PC_page,frameNum,victimframe);
            fclose(fp);
        }
        // Update PC and PC_offset of that pcb
        pcb->PC = pcb->pageTable[pcb->PC_page]*4;
        pcb->PC_offset = 0;
        // add it again to the ready queue
        addToReady(pcb);
    }
    return 0;
}


int scheduler(){
    // set CPU quanta to default, IP to -1, IR = NULL
    CPU.quanta = DEFAULT_QUANTA;
    CPU.IP = -1;
    while (size() != 0){
        //pop head of queue
        PCB* pcb = pop();

        //Initialise the IP and offset of the CPU
        // CPU.IP = pcb -> IP???????????????????
        CPU.IP = pcb->PC;
        CPU.offset = pcb->PC_offset;
        int errorCode = run(CPU.quanta);

        int isOver = FALSE;
        int isPageFault = FALSE;

        // Either end of program or page fault
        if (errorCode == 1 || errorCode == 2){
            // if last page, it is end of program
            if (pcb->PC_page == pcb->pages_max-1){
                isOver = TRUE;
            // else pageFault
            } else {
                isPageFault = TRUE;
            }
        }

        // If an error occurred or the program is done, delete frames of that pcb and free it
        if ( errorCode<0 || isOver ){
            //free the frames occupied by this pcb
            freeFramesForPCB(pcb);
            //Deallocate the memory for the pcb pointer
            free(pcb);
        // If a page fault occurred, 
        } else if (isPageFault)  {
            resolvePageFault(pcb);
        // If no error occured, increment the page_offset of that pcb
        } else {
            pcb->PC_offset+=2;
            addToReady(pcb);
        }
    }
    // Clear RAM?
    return 0;
}

/*
Flushes every pcb off the ready queue in the case of a load error
*/
void emptyReadyQueue(){
    while (head!=NULL){
        ReadyQueueNode * temp = head;
        head = head->next;
        free(temp->PCB);
        free(temp);
    }
    sizeOfQueue =0;
}

/*
Given a frameNumber, it returns the PCB that was allocated
this frame.
*/
PCB* getFrameOwner(int frameNumber){
    ReadyQueueNode * temp = head;
    while (temp!=NULL){
        if (isAFrameOf(temp->PCB,frameNumber)) return temp->PCB;
        temp = temp->next;
    }
}

