CC = gcc
CFLAGS = -Wall -std=c99 -pedantic -g
MAIN = file_emulator
OBJS = file_emulator.c

all : $(MAIN)

file_emulator.o : file_emulator.c
	$(CC) $(CFLAGS) -c file_emulator.c

clean :
	rm *.o $(MAIN) core