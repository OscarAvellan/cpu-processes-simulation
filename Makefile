# Created by Oscar Avellan

## CC  = Compiler.
## CFLAGS = Compiler flags.
CC	= gcc
CFLAGS =	-Wall -Wextra# -std=gnu99


## OBJ = Object files.
## SRC = Source files.
## EXE = Executable name.

SRC =		swap.c processes.c list.c
OBJ =		swap.o processes.o list.o
EXE = 		swap

## Top level target is executable.
$(EXE):	$(OBJ)
		$(CC) $(CFLAGS) -o $(EXE) $(OBJ)


## Clean: Remove object files and core dump files.
clean:
		/bin/rm $(OBJ)

## Clobber: Performs Clean and removes executable file.

clobber: clean
		/bin/rm $(EXE)

## Dependencies

swap.o:		list.h processes.h
processes.o:	processes.h list.h
list.o: 	list.h
