CC = gcc -g
CFLAGS = -lpthread -lapue -lList
HOME = ..
HEADERS = -I$(HOME)/headers
LIBS	= -L$(HOME)/lib

install:server client
	rm *.o
client: client_2.c
	$(CC) -o $@ client_2.c $(LIBS) $(HEADERS) $(CFLAGS)
server:	michat.o #List.o
	$(CC) -o $@ michat.o $(LIBS) $(CFLAGS) 
michat.o:	michat.c
	$(CC) -c -o $@ michat.c $(HEADERS) 
#List.o:	List.c
#	$(CC) -c -o $@ List.c $(HEADERS) 
clean:
	-rm server client
