/*
Loads the page (corresponding to the pagenumber) from the file 
into the frame (corresponding to the framenumber) in the ram
*/
void loadPage(int pageNumber, FILE* fp, int frameNumber);

/*
Looks for a free frame (4 consecutive cell set to NULL) starting from the first Frame in RAM,
and checking the next frame and so on.
If no such free frame exist, return -1
Else a free frame was found, return its framenumber
*/
int findFrame();

/*
Returns the frame number that will be allocated to that PCB passed.
This frame will be taken from another PCB. Note that the victim will
be a PCB whose current/active frame is other than the frame number returned
*/
int findVictim(struct PCB* p);



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
int updatePageTable(struct PCB* p, int pageNumber, int frameNumber, int victimFrame);

/*
Returns 1 for success, 0 for error
*/
int launcher(FILE* fp1);

/*
Free the frames belonging to the passed PCB from the RAM
*/
int freeFramesForPCB(struct PCB* pcb);