#!/bin/bash

#Compile the o files
gcc -c kernel.c cpu.c ram.c shell.c shellmemory.c interpreter.c pcb.c memorymanager.c
#Compile the executable mykernel
gcc -o mykernel shellmemory.o kernel.o cpu.o ram.o shell.o interpreter.o pcb.o memorymanager.o
#remove all .o files
rm *.o
exit