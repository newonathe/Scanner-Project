all: scanit.exe compute.exe

scanit.exe: scanit.o scan.o token.o
	gcc -o scanit.exe scanit.o scan.o token.o

compute.exe: compute.o scan.o token.o
	gcc -o compute.exe compute.o scan.o token.o

scanit.o: scanit.c scan.h token.h
	gcc -c scanit.c

compute.o: compute.c scan.h token.h
	gcc -c compute.c

scan.o: scan.c scan.h token.h
	gcc -c scan.c

token.o: token.c token.h
	gcc -c token.c
