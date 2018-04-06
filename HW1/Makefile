CC = arm-none-linux-gnueabi-gcc
CFAGS = --static -o -W -Wall
TARGET = 20131612
OBJECTS = reader.o process.o

all : $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

clean :
	rm *.o 20131612
