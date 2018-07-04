
main:main.o Socket.o
	gcc -o main main.o Socket.o 

main.o:main.c Socket.h 
	gcc -c main.c

Socket.o:Socket.c Socket.h
	gcc -c Socket.c
