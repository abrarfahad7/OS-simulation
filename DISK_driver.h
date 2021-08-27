char partitionFile[50];

void initIO();
int partition(char *name, int totalblocks, int blocksize);
int mountFS(char *name);

int openfile(char *name);
char *readBlock(int file);

int writeBlock(int file, char *data);
void close_file(int file);
int returnBlockLength();