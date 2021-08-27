#include<stdio.h>
#include<string.h>
/*
Stores the index of the next available block of cell
*/
int nextFree = 0;

/*
Ram structure implemented as an array.
Size is 40 strings
*/
char *ram[40]; 

/*
This function will delete the content in ram between the 2 indices parameters (inclusive)
start : pointer to variable which will store the index of first line of file in ram
end : pointer to variable which will store the index of last line of file in ram
*/
void removeFromRam (int start, int end){
    for (int i = start; i <= end; i++)
    {
        ram[i] = NULL;
    }
}

/*
This function clears the RAM.
*/
void clearRAM (){
    removeFromRam(0,39);
}

/*
This function will delete a frame corresponding to the passed frameNumber from the ram
*/
void removeFrameFromRAM(int frameNumber){
    int start = frameNumber*4;
    int end = start + 4;
    removeFromRam(start,end);
}

/*
This function will add the content of a file to the local ram array variable
In the case of an error (due to lack of RAM), -1 will be assigned to
the values pointed by start and end. Use this to check for errors.
p : file pointer
start : pointer to variable which will store the index of first line of file in ram
end : pointer to variable which will store the index of last line of file in ram
*/
void addToRAM (FILE *p, int *start, int *end){
    *start = nextFree;
    int i = *start;
    char buffer[1000];
    // Copy content of the file while
    // we have not reached the end of the file
    // and RAM is not full yet.
    while (!feof(p) && i<40){
        fgets(buffer,999,p);
        ram[i]= strdup(buffer);
        i++;
    }
    // If RAM is full and we have not reached the end of the 
    // file, remove the content from RAM and return -1 in *start,*end
    // Update nextFree as well
    
    if (i>=40 && !feof(p)){
        removeFromRam(0,i-1);
        nextFree = 0;
        *start = -1;
        *end = -1;        
    // Else update the nextFree spot and *end
    } else {
        nextFree=i;
        *end=i-1;
    }
}

/*
Reset the pointer to the free cell back to index 0
*/
void resetRAM(){
    nextFree = 0;
}

/*
Passing a filepointer to a line in a textfile, the function will read the 4 lines from that pointer
and store these 4 lines at the framenumber specified.
*/
void addFrameToRAM(FILE *p, int frameNumber){
    char buffer[1000];
    int i=0;
    int pos = frameNumber*4;
    while (fgets(buffer,999,p)!=NULL && i<4){
        ram[pos]= strdup(buffer);
        pos++;
        i++;
    }
    //If file ends prematurely, set the remaining cells to NULL;
    for (int j = i; i < 4; i++)
    {
        ram[pos] = NULL;
    }
}

/*
Helper function for debugging purposes. 
Display the whole ram in Frames
*/
void displayRAM(){

    printf("RAM CONTENT:\n\n");
    for (int i = 0; i < 40; i++)
    {
        if (i%4==0){
            printf("--Frame %d--\n",i/4);
        }
        if (ram[i]==NULL){
            printf("%d: NULL\n", i);
        } else {
            printf("%d: %s",i,ram[i]);
        }
    }
    printf("----------\n");
}