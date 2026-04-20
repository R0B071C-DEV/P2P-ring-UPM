CFLAGS=-Wall

PROGRS=client_sendmore client_writev server copy_sendfile copy_read_mmap

all:  $(PROGRS)

client_sendmore.o client_writev.o server.o common.o: common.h

client_sendmore: client_sendmore.o common.o
	$(CC) -o $@ $^

client_writev: client_writev.o common.o
	$(CC) -o $@ $^

server: server.o common.o
	$(CC) -o $@ $^ -lpthread

clean:
	rm -f $(PROGRS) *.o
