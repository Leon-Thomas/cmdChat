CC = gcc -g
CFLAGS = -lpthread -lapue -lList
HOME = ..
HEADERS = -I$(HOME)/headers
LIBS	= -L$(HOME)/lib

install:server client
	rm *.o
client: client_2.c
	$(CC) -o $@ client_2.c $(LIBS) $(HEADERS) $(CFLAGS)
server:	server.o 
	$(CC) -o $@ server.o $(LIBS) $(CFLAGS) 
server.o:	chatServer.c
	$(CC) -c -o $@ chatServer.c $(HEADERS) 

clean:
	-rm server client
