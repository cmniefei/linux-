.SUFFIXES: .c .o

CC=gcc
	
SERVERSRCS=server.c
CLIENTSRCS=client.c

SERVEROBJS=$(SERVERSRCS:.c=.o)

CLIENTOBJS=$(CLIENTSRCS:.c=.o)

SERVEREXEC=server
CLIENTEXEC=client

all: $(SERVEROBJS) $(CLIENTOBJS) pub.o
	$(CC)  -o $(SERVEREXEC) $(SERVEROBJS) pub.o
	$(CC)  -o $(CLIENTEXEC) $(CLIENTOBJS) -lpthread
	@echo '-------------ok--------------'

.c.o:
	$(CC) -Wall -l . -g -o $@ -c $< 

clean:
	rm -f *.o
	rm $(SERVEREXEC) $(CLIENTEXEC)

