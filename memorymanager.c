#include <string.h>
#include <stdio.h>
#include<math.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "pcb.h"
#include "ram.h"
#include "interpreter.h"
#include "kernel.h"

//number of pcb/processes launched so far (EXEC)
int no_process=0;

int countTotalPages(FILE* fp){
	int no_lines=0;
    char buffer[1000];

    while (!feof(fp)){
        fgets(buffer,999,fp);
        no_lines++;
    }
    rewind(fp);

    int no_pages = no_lines/4;
    if (no_lines%4 != 0) no_pages++;
    return no_pages;
}

/*
Loads the page (corresponding to the pagenumber) from the file 
into the frame (corresponding to the framenumber) in the ram
*/
void loadPage(int pageNumber, FILE* fp, int frameNumber){

	//resets file pointer
	rewind(fp);
    //Moves file pointer to the required page
	char buffer[1000];
	int i = 0;
	while (i<pageNumber*4){
		fgets(buffer,999,fp);
		i++;
	}
    // Adds that page to the required frame in RAM
    addFrameToRAM(fp,frameNumber);
    //Reset file pointer to the beginning of the while
    rewind(fp);
}

/*
Looks for a free frame (4 consecutive cell set to NULL) starting from the first Frame in RAM,
and checking the next frame and so on.
If no such free frame exist, return -1
Else a free frame was found, return its framenumber
*/
int findFrame(){
    int found = FALSE;
    int frame = -1;
    //For each consecutive frame
    for (int i=0; i< 10; i++){
        // If 1st cell in that frame is null, examine the 3 other cells
        if (ram[i*4]==NULL){
            int found = TRUE;
            //examining other 3 cells
            for (int j = i*4+1; j < i*4+4; j++)
            {
                if (ram[j] != NULL){
                    found = FALSE;
                    break;
                }
            }

            // If found, initialise frame
            if (found) {
				frame = i;
                break;
            }
        }
    }
    return frame;
}

/*
Returns the frame number that will be allocated to that PCB passed.
This frame will be taken from another PCB. Note that the victim will
be a PCB whose current/active frame is other than the frame number returned
*/
int findVictim(struct PCB* p){
	//get a random framenumber
	int frameNum = rand()%10;
	// loop ends when we find a frame which is not the current/active frame
	// of its owner pcb (victim)
	while (TRUE){
		//while the current frame belong to the pcb, check the next one
		while(isAFrameOf(p,frameNum)){
			frameNum=(frameNum+1)%10;
		}
		// get the pcb (victim) to which this frame belongs
		struct PCB* victim = getFrameOwner(frameNum);
		// if this frame is not the current/active frame of the victim pcb, we return this frameNumber
		if (victim->pageTable[victim->PC_page] != frameNum){
			return frameNum;
		// else loop again checking the next frame
		} else {
			frameNum=(frameNum+1)%10;
		}
	}
}

/*
Passes 4 arguments
p : PCB whose current page is being allocated a frame to
pageNumber : pageNumber of that page
frameNumber : the frameNumber of the frame allocated to this page
victimFrame : If equal to -1, nothing
			  If not equal to -1, then it is the same as the frameNumber
			  but denotes that this frame belongs to another PCB being the victim
			  so we need to change the pageTable of the victim PCB
Updates the pageTable of the passed PCB
*/
int updatePageTable(struct PCB* p, int pageNumber, int frameNumber, int victimFrame){
    // If we have a victim
	if(victimFrame != -1){
        // Find the victim pcb
		struct PCB* victim = getFrameOwner(victimFrame);
        // Find which page lost its frame and set its value to -1 
		for(int i=0;i<10;i++){
			if(victim->pageTable[i]==victimFrame){ 
				victim->pageTable[i]=-1; 
			}
		}
	}
	//Update to which frame the page of the current pcb will now point to
	p->pageTable[pageNumber]=frameNumber;
	return 0;
}

/*
Returns 1 for success, 0 for error
*/
int launcher(FILE* fp1){
	//Creating the file in the backing store
	char newProcess[50] = "BackingStore/file";
	char* extension = ".txt";
	char buffer[8];
	sprintf(buffer,"%d",no_process);
	strcat(newProcess,buffer);
	strcat(newProcess,extension);

	//Opening the newly created file
	FILE* fp2 = fopen(newProcess,"w+");

	//If file cannot be opened, return error 0
	if (fp2 == NULL){
		return 0;
	}

	//Copying content of file passed into the newly created file in the backing store
	char c;
	c = fgetc(fp1);
	while(c != EOF){
		fputc(c,fp2);
		c = fgetc(fp1);
	}

	//Close file opened for reading
	fclose(fp1);
	//Resets pointer to the backing store file
	rewind(fp2);

	// Getting the max pages for that program and creates a PCB for it
	int no_page = countTotalPages(fp2);
	struct PCB* pcb = makePCB(no_page,no_process);

	// if cannot create PCB, close file in backing store and return 0;
	if (pcb == NULL){
		fclose(fp2);
		return 0;
	}

	no_process++;

	// Finding a frame for 1st page
	int frame = findFrame();
	int victimFrame = -1;
	if(frame == -1){//if so, we need to find victim
		victimFrame = findVictim(pcb);
		frame = victimFrame;
	}
	
	//Clear memory at that frame
	removeFrameFromRAM(frame);
	//loads first page at that frame
	loadPage(0, fp2, frame);
	//Update pageTable
	updatePageTable(pcb,0,frame,victimFrame);//updates pcb's page table
	// Sets PC to first line in RAM of that frame
	pcb->PC = frame*4;
	
	// If program requires more than 1 page, load the 2nd page.
	if(no_page>1){

		//Finding a frame for the 2nd page
		frame = findFrame();
		victimFrame = -1;
		if(frame == -1){
			victimFrame = findVictim(pcb);
			frame = victimFrame;
		}
		//Clear memory at that frame
		removeFrameFromRAM(frame);
		//loads first page at that frame
		loadPage(1, fp2, frame);
		//Update pageTable
		updatePageTable(pcb,1,frame,victimFrame);//updates pcb's page table
	}

	//close backing store file
	fclose(fp2);
	//This pcb is now added into the ready queue as its first 2 pages are loaded in RAM.
	addToReady(pcb);
	return 1;
}

/*
Free the frames belonging to the passed PCB from the RAM
*/
int freeFramesForPCB(struct PCB* pcb){
	for (int i = 0; i < pcb->pages_max; i++)
	{
		// if the current page has a frame in the RAM, remove it
		if (pcb->pageTable[i] != -1){
			removeFrameFromRAM(pcb->pageTable[i]);
			pcb->pageTable[i] = -1;
		}
	}
	return 0;
}