
CC = gcc

TARGET = packImage

SRCS = packImage.c

CFLAGS += -static -m32

all:
	gcc -o $(TARGET) $(CFLAGS) $(SRCS)
	strip -s $(TARGET)

clean:
	rm -rf $(TARGET) *.o

