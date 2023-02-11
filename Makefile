CC=gcc
CFLAGS=-Wall -Iincludes -Wextra -ggdb
PROGS= client server

all: $(PROGS)

client: client.o a1protos.o optparser.o util.o
	$(CC) -o client client.o a1protos.o optparser.o util.o

server: server.o optparser.o util.o a1protos.o
	$(CC) -o server server.o optparser.o util.o a1protos.o

client.o: client.c a1protos.h optparser.h util.h
	$(CC) $(CFLAGS) -c client.c

server.o: server.c optparser.h util.h
	$(CC) $(CFLAGS) -c server.c

a1protos.o: a1protos.c a1protos.h

util.o: util.c util.h

optparser.o: optparser.c optparser.h

clean:
	@echo "Removing object and exe files"
	rm -f *.o $(PROGS)

.PHONY : clean all
