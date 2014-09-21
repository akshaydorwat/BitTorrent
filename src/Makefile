CC=gcc
CPFLAGS=-g -Wall
LDFLAGS= -lcrypto


SRC= bt_client.c bt_lib.c bt_setup.c Server.cpp
OBJ=$(SRC:.c=.o)
BIN=bt_client

all: $(BIN)

$(BIN): $(OBJ)	
	$(CC) $(CPFLAGS) $(LDFLAGS) -o $(BIN) $(OBJ) 


%.o:%.c
	$(CC) -c $(CPFLAGS) -o $@ $<  

$(SRC):

clean:
	rm -rf $(OBJ) $(BIN)
