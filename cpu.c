#include<string.h>
#include"ram.h"
#include"interpreter.h"
#include"shell.h"

#define DEFAULT_QUANTA 2

struct CPU { 
    int IP; 
    int offset;
    char IR[1000]; 
    int quanta; 
} CPU;

/*
This method passes a quanta.
It will then executes quanta number of lines in RAM as from the address
pointed by the CPU IP.

Returns an status code:
1 : Program is over during the quantas
2 : Page Fault (Need to check if it's the last page. 
    If it is, program is over.
    Else, perform swap in)
Or
If error, returns an error code (<1)
Or
Otherwise, returns 0
*/
int run(int quanta){
    for (int i = 0; i < quanta; i++)
    {
        int eff_address = CPU.IP + CPU.offset;

        //If this cell with our frame in RAM is NULL, then we executed the last instruction of the program
        // Return status code 1 to indicate end of program
        if (ram[eff_address] == NULL){
            return 1;
        }

        strcpy(CPU.IR,ram[eff_address]);
        int errorCode = parse(CPU.IR);
        // Do error checking and if error, return error
        if (errorCode != 0){
            // Display error message if fatal
            if (errorCode < 0){
                ram[CPU.IP][strlen(ram[CPU.IP])-2]='\0';
                displayCode(errorCode,ram[CPU.IP]);
            }
            return errorCode;
        }
        // Increment offset
        CPU.offset ++;

        // If offset = 4, page fault occurred so return 2
        if ( CPU.offset == 4){
            return 2;
        }
    }
    return 0;
}
