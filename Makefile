CC = gcc
CFLAGS = -std=c99 -Wall -Wextra

all: scanit.exe compute.exe

token.o: token.c token.h
	$(CC) $(CFLAGS) -c token.c

scan.o: scan.c scan.h token.h
	$(CC) $(CFLAGS) -c scan.c

inputs.o: inputs.c inputs.h
	$(CC) $(CFLAGS) -c inputs.c

scanit.o: scanit.c scan.h token.h inputs.h
	$(CC) $(CFLAGS) -c scanit.c

compute.o: compute.c scan.h token.h inputs.h
	$(CC) $(CFLAGS) -c compute.c

scanit.exe: scanit.o scan.o token.o inputs.o
	$(CC) -o scanit.exe scanit.o scan.o token.o inputs.o

compute.exe: compute.o scan.o token.o inputs.o
	$(CC) -o compute.exe compute.o scan.o token.o inputs.o
