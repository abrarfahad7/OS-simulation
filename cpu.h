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
int run(int);