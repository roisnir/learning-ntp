CC=gcc
CFLAGS=-fdiagnostics-color=always -g -Wall -pedantic

all: ntp_client

ntp_client: ntp_client.o utils.o ntp.o
	$(CC) $(CFLAGS) -o $@ $^ -lm

stuff: stuff.c
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@ -lm

clean:
	$(RM) ./a.out ./stuff ./ntp_client ./*.o ./*.gch