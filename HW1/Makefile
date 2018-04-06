CC = arm-none-linux-gnueabi-gcc --static
#CC = gcc
CFAGS = --static
TARGET = 20131612
OBJECTS = reader.o process.o writer.o

all : $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) -o $@ $^

clean :
	rm *.o 20131612
