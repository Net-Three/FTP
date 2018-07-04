
main:main.o CreatServer.o
	gcc -pthread -o main main.o CreatServer.o 

main.o:main.c CreatServer.h 
	gcc -c main.c

CreatServer.o:CreatServer.c CreatServer.h
	gcc -c CreatServer.c
