CC=g++
CPFLAGS=-g -Wall
LDFLAGS= -lcrypto -pthread


VPATH = src
OBJ = bt_client.o bt_lib.o bt_setup.o Server.o threadpool.o
BIN = bt_client

all: $(BIN)

$(BIN): $(OBJ)	
	$(CC) $(CPFLAGS)  -o $(BIN) $(OBJ) $(LDFLAGS)

%.o:%.c
	$(CC) -c $(CPFLAGS) -o $@ $<  

%.o:%.cpp
	$(CC) -c $(CPFLAGS) -o $@ $<  


clean:
	rm -rf $(OBJ) $(BIN)
