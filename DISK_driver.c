#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <ctype.h>
#include "shell.h"

struct PARTITION {
    int total_blocks; // block count
    int block_size;   // size in bytes of a block, 1 byte = 1 character
    int latest_block;
} my_partition;

struct FAT {
    char *filename;       // name of file in the partition
    int file_length;      // length of file in number of blocks, 1 block = blockSize * byte(char)
    int blockPtrs[10];    // pointers to the blocks in the partition
    int current_location; // pointer to the current read/write location of the file in block number
} fat[20];                // initialized to -1

char *block_buffer;

FILE *active_file_table[5];
int active_files[5];
char partitionFile[50];

// initialize all global data structure and variables to zero or null. 
// Called from your boot() function.

void initIO(){
    
    my_partition.total_blocks = 0;
    my_partition.block_size = 0;
    my_partition.latest_block = -1;

    for (int i = 0; i < 20; i++){
        fat[i].filename = "NULL";
        fat[i].file_length = 0;
        fat[i].current_location = 0;

        for (int j = 0; j < 10; j++){
            fat[i].blockPtrs[j] = -1;
        }
    }

    for (int i = 0; i < 5; i++){
        active_file_table[i] = NULL;
        active_files[i] = -1;
    }

}

// create & format partition. Called from your mount() function 
// that lives in the interpreter, associated to your scripting mount command.
int returnBlockLength(){
    return my_partition.block_size;
}

int partition(char *name, int blocksize, int totalblocks){

    block_buffer = malloc(sizeof(char)*blocksize);
    my_partition.block_size = blocksize;
    my_partition.total_blocks = totalblocks;
    my_partition.latest_block = 0;

    DIR* dir = opendir("PARTITION");
    // if directory exists, close it and remove its content
    if (dir) {
        closedir(dir);
        system("rm -r PARTITION/*");
    //if directory does not exist, create it
    } else if (ENOENT == errno) {
        system("mkdir PARTITION");
    }


    //Within PARTITION create a file having the name argument as its filename.

    char filePath[50] = "PARTITION/";
    strcat(filePath, name);

    strcpy(partitionFile, filePath);


    FILE *fp = fopen(filePath, "w+");

    if (fp == NULL){
        printf("Could not open file!\n");
        displayCode(-7, "");
        return 0;
    }

    fprintf(fp, "%d %d :\n", blocksize, totalblocks);

    for (int i = 0; i < 20; i++){
        char fatLine[100];

        sprintf(fatLine, "%s %d %d %d %d %d %d %d %d %d %d %d %d",
        fat[i].filename, fat[i].file_length, fat[i].blockPtrs[0], fat[i].blockPtrs[1],
        fat[i].blockPtrs[2], fat[i].blockPtrs[3], fat[i].blockPtrs[4], fat[i].blockPtrs[5],
        fat[i].blockPtrs[6], fat[i].blockPtrs[7], fat[i].blockPtrs[8], fat[i].blockPtrs[9],
        fat[i].current_location);

        fprintf(fp, "%s\n", fatLine);

    }

    int partitionSize = blocksize * totalblocks;
    for (int i = 0; i < partitionSize; i++){
        fprintf(fp, "%s", "0");
    }

    //fprintf(fp, "\n");
    fclose(fp);

    return 1;

}

// load FAT & create buffer_block. Called from your mount() function 
// that lives in the interpreter, associated to your scripting mount command

int mountFS(char *name){
    // It opens the partition from the PARTITION directory and loads the 
    // information from the partition into the global structures
    // partition and fat[]

    char filePath[50] = "PARTITION/";
    strcat(filePath, name);

    FILE *fp = fopen(filePath, "r");

    // Read the formatting from the partition and store the values into
    // partition and FAT
    int cur_line = 0;
    char line[100];
    int fat_index = 0;
    int counter = 0;

    while (fgets(line, sizeof(line), fp) && counter < 20){
          
        if (isdigit(line[0]) && cur_line == 0){   // for the first line of file concerning block size
            char *buffer[2];
            char *p = strtok(line, " ");
            int i = 0;

            while (p != NULL && strcmp(p, ":") != 0){
                buffer[i++] = p;
                p = strtok(NULL, " ");
            }

            my_partition.block_size = atoi(buffer[0]);
            my_partition.total_blocks = atoi(buffer[1]);

            //printf("%d %d\n", my_partition.block_size, my_partition.total_blocks);

            cur_line++;
        }

        

        if (isalpha(line[0]) && fat_index < 20){
            
            char *buffer[13];
            char *p = strtok(line, " ");
            int i = 0;

            while (p != NULL){
                buffer[i++] = p;
                p = strtok(NULL, " ");
            }

            fat[fat_index].filename = malloc(sizeof(&buffer[0]));
            strcpy(fat[fat_index].filename, buffer[0]);

            fat[fat_index].file_length = atoi(buffer[1]);


            for (int j = 0; j < 10; j++){
                fat[fat_index].blockPtrs[j] = atoi(buffer[2+j]);
            }

            fat[fat_index].current_location = atoi(buffer[12]);
            
            fat_index++;
            

        }
        counter++;
        
    }

    fclose(fp);

    // TODO: The function will also malloc 
    // block_size bytes and assign that to block_buffer
    return 1;
}

//find filename or creates file if it does not exist, returns file’s
// FAT index. Called from your scripting read and write commands in the interpreter

int openfile(char *name){

    if (!(my_partition.latest_block < my_partition.total_blocks)){
        return -1;
    }

    int availableSlot = 0;
    int i = 0;
    for (; i < 20; i++){
        if (strcmp(fat[i].filename, name) == 0 || fat[i].filename == NULL){
            break;
        }
    }


    availableSlot = i;

    for (int j = 0; j < 20; j++){
        if (active_files[j] == availableSlot){
            return availableSlot;
        }
    }

    int index;

    for (index = 0; index < 20; index++){
        if(active_file_table[index] == NULL){
            break;
        }
    }

    if (index == 20){
        return -1;
    }

    active_file_table[index] = fopen(name, "r+");
    active_files[index] = availableSlot;


    if (availableSlot == 20){
        
        if (fat[19].filename != NULL){
            
            fclose(active_file_table[index]); 
            
            active_file_table[index] = NULL;
            active_files[index] = -1;

            
            
            return -1;
        }
    }

    else if (fat[availableSlot].filename == NULL){
        
        fat[availableSlot].filename = malloc(sizeof(500));
        strcpy(fat[availableSlot].filename, name);
        fat[availableSlot].current_location = 0;
        fat[availableSlot].file_length = 0;
    }

    else{
        
        fat[availableSlot].current_location = 0;
    }

    return availableSlot;
}

// using the file FAT index number, load buffer with data from current_location. 
// Return block data as string from block_buffer.

char *readBlock(int file){

    if (file < 0 || fat[file].current_location >= 10){
        return NULL;
    }

    int cur_block = fat[file].blockPtrs[fat[file].current_location];

    if (cur_block = -1){
        return NULL;
    }

    FILE *fp = NULL;
    int index;
    int i;

    for (i = 0; i < 20; i++){
        if (active_files[i] == file){
            fp = active_file_table[i];
            break;
        }
    }

    index = i;

    if (index == 20){
        return NULL;
    }

    //move to data blocks

    fseek(fp, 0, SEEK_SET);
    char line[100];
    int counter = 0;

    while (fgets(line, sizeof(line), fp) && counter <= 21){
        counter++;
    }

    for (int i = cur_block * my_partition.block_size; i > 0; i--){
        fgetc(fp);
    }

    fseek(fp, 0, SEEK_CUR);

    int current = 0;

    for (; current < my_partition.block_size; i++){
        char c = fgetc(fp);
        if (c == '0'){
            block_buffer[i] = '\0';
            break;
        }

        block_buffer[i] = c;
    }

    block_buffer[i] = '\0';
    fat[file].current_location++;
    return block_buffer;

}

// using the file FAT index number, write data to disk at current_location

void update_FAT_entries(int file){
    fat[file].blockPtrs[fat[file].current_location] = my_partition.latest_block;
    my_partition.latest_block++;
    fat[file].current_location++;
    fat[file].file_length++;

    if (fat[file].current_location < 20){
        fat[file].blockPtrs[fat[file].current_location] = -1;
    }
}

int writeBlock(int file, char *data){
    if (file < 0 || fat[file].current_location >= 10){
        return 0;
    }

    fat[file].current_location = fat[file].file_length;

    FILE *fp = NULL;
    int index;
    int i;

    for (i = 0; i < 20; i++){
        if (active_files[i] == file){
            fp = active_file_table[i];
            break;
        }
    }

    index = i;

    if (index == 20){
        return 1;
    }

    //move to data blocks

    fseek(fp, 0, SEEK_SET);
    char line[100];
    int counter = 0;

    while (fgets(line, sizeof(line), fp) && counter <= 21){
        counter++;
    }

    for (int i = my_partition.latest_block * my_partition.block_size; i > 0; i--){
        fgetc(fp);
    }

    fseek(fp, 0, SEEK_CUR);

    for (int i = 0; i < my_partition.block_size; i++){
        char c = *(data+i);

        if (c == 0){
            c = '0';
        }

        fputc(c, fp);
    }

    update_FAT_entries(file);

    FILE *partit = fopen(partitionFile, "r+");
    fprintf(fp, "%d %d :\n", my_partition.block_size, my_partition.total_blocks);

    for (int i = 0; i < 20; i++){
        char fatLine[100];

        sprintf(fatLine, "%s %d %d %d %d %d %d %d %d %d %d %d %d",
        fat[i].filename, fat[i].file_length, fat[i].blockPtrs[0], fat[i].blockPtrs[1],
        fat[i].blockPtrs[2], fat[i].blockPtrs[3], fat[i].blockPtrs[4], fat[i].blockPtrs[5],
        fat[i].blockPtrs[6], fat[i].blockPtrs[7], fat[i].blockPtrs[8], fat[i].blockPtrs[9],
        fat[i].current_location);

        fprintf(fp, "%s\n", fatLine);

    }

    fclose(partit);

    return 1;
}

void close_file(int file){

    int i;
    int index = 0;

    for (i = 0; i < 20; i++){
        if(active_files[i] == file){
            break;
        }
    }

    index = i;

    if (index == 20){
        return;
    }

    fclose(active_file_table[i]);
    active_files[i] = -1;

    return;
}