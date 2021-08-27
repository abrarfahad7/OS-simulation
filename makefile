mykernel: kernel.o cpu.o ram.o shell.o interpreter.o pcb.o shellmemory.o memorymanager.o DISK_driver.o
	gcc -o mykernel kernel.o cpu.o ram.o shell.o interpreter.o pcb.o shellmemory.o memorymanager.o DISK_driver.o
kernel.o: kernel.c kernel.h
	gcc -c kernel.c
cpu.o: cpu.c cpu.h
	gcc -c cpu.c
ram.o: ram.c ram.h
	gcc -c ram.c
shell.o: shell.c shell.h
	gcc -c shell.c 
interpreter.o: interpreter.c interpreter.h
	gcc -c interpreter.c 
pcb.o: pcb.c pcb.h
	gcc -c pcb.c 
shellmemory.o: shellmemory.c shellmemory.h
	gcc -c shellmemory.c 
memorymanager.o: memorymanager.c memorymanager.h
	gcc -c memorymanager.c 
DISK_driver.o: DISK_driver.c
	gcc -c DISK_driver.c
clear: 
	rm *.o
