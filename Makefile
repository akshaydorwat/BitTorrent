CC=g++
CPFLAGS=-g -Wall
LDFLAGS= -lcrypto -pthread


VPATH = src
OBJ = bt_client.o bt_lib.o bt_setup.o Reactor.o threadpool.o
BIN = bt_client

all: $(BIN)

$(BIN): $(OBJ)	
	$(CC) $(CPFLAGS)  -o $(BIN) $(OBJ) $(LDFLAGS)

%.o:%.c
	$(CC) -c $(CPFLAGS) -o $@ $<  

%.o:%.cpp
	$(CC) -c $(CPFLAGS) -o $@ $<  

run:
	./bt_client -v -p 10.0.0.88:6767 -p 10.0.0.2:6767 -s . sample/download.mp3.torrent
clean:
	rm -rf $(OBJ) $(BIN)
