CC = arm-none-linux-gnueabi-gcc --static
#CC = gcc
#CFAGS = --static
#TARGET = reader
#OBJECTS1 = reader.o process.o writer.o

all : process reader writer
process : process.o
	$(CC) -o process process.o
reader : reader.o
	$(CC) -o reader reader.o
writer : writer.o
	$(CC) -o writer writer.o

#reader.o : reader.c reader.h
#$(CC) -o reader.o reader.c

#process.o : process.c process.h
#$(CC) -o process.o process.c



clean :
	rm -i *.o reader process writer
