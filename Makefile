#Author: Sumbul Aftab
CC=clang -Wall
SERVERSRC = Server_BS.c
SERVEREXE = Server_BS
CLIENTSRC = Client_BS.c
CLIENTEXE = Client_BS
all: server client
server:Server_BS.c
	$(CC) $(SERVERSRC) -o $(SERVEREXE) -L/usr/local/lib -lczmq -lzmq
client:Client_BS.c
	$(CC) $(CLIENTSRC) -o $(CLIENTEXE) -L/usr/local/lib -lczmq -lzmq
clean:
	-rm -f Server_BS Client_BS
