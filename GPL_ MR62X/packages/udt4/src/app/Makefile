
ifndef os
   os = LINUX
endif


LDFLAGS += -L../src -ludt -lstdc++ -lpthread -lm
CFLAGS += -I../src


DIR = $(shell pwd)

APP = appserver appclient sendfile recvfile test

all: $(APP)

%.o: %.cpp
	$(CC) $(CFLAGS) $< -c

appserver: appserver.o
	$(CC) $^ -o $@ $(LDFLAGS)
appclient: appclient.o
	$(CC) $^ -o $@ $(LDFLAGS)
sendfile: sendfile.o
	$(CC) $^ -o $@ $(LDFLAGS)
recvfile: recvfile.o
	$(CC) $^ -o $@ $(LDFLAGS)
test: test.o
	$(CC) $^ -o $@ $(LDFLAGS)

clean:
	rm -f *.o $(APP)

