ARP_SERV = dslite
ARP_LDFLAGS = $(LDFLAGS)
ARP_CFLAGS = $(CFLAGS)

all: $(ARP_SERV)
OBJ = dslite.o

$(ARP_SERV):$(OBJ)
	$(CC) -o $@ $(ARP_LDFLAGS) $^ 

%.o: %.c
	@echo Compile $<
	$(CC) $(ARP_CFLAGS) -c -o $@ $<

clean:
	rm -rf *.o $(ARP_SERV)
