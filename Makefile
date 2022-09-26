CC=gcc
CFLAGS=-fdiagnostics-color=always -g -Wall -pedantic

all: ntp_client

ntp_client: ntp_client.o
	$(CC) $(CFLAGS) -o $@ $^

stuff: stuff.c
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@

clean:
	$(RM) ./a.out ./stuff ./ntp_client ./*.o ./*.gch