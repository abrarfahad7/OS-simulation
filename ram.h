#include<stdio.h>

/*
Ram structure implemented as an array.
Size is 40 strings
*/
char *ram[40];

/*
This function will add the content of a file to the local ram array variable
In the case of an error (due to lack of RAM), -1 will be assigned to
the values pointed by start and end. Use this to check for errors.
p : file pointer
start : pointer to variable which will store the index of first line of file in ram
end : pointer to variable which will store the index of last line of file in ram
*/
void addToRAM (FILE *, int *, int *);

/*
This function will delete the content in ram between the 2 indices parameters (inclusive)
start : pointer to variable which will store the index of first line of file in ram
end : pointer to variable which will store the index of last line of file in ram
*/
void removeFromRam (int, int);

/*
This function will delete a frame corresponding to the passed frameNumber from the ram
*/
void removeFrameFromRAM(int);

/*
This function clears the RAM.
*/
void clearRAM ();

/*
Passing a filepointer to a line in a textfile, the function will read the 4 lines from that pointer
and store these 4 lines at the framenumber specified.
*/
void addFrameToRAM(FILE *p, int frameNumber);

void displayRAM();