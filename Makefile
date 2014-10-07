CC=g++
CPFLAGS=-g -Wall -std=c++0x 
LDFLAGS= -lcrypto -pthread

VPATH = src

OBJ = \
bt_client.o \
bt_lib.o \
bt_setup.o \
Reactor.o \
Logger.o \
Peer.o \
Bencode_t.o \
BencodeInteger_t.o \
BencodeString_t.o \
BencodeList_t.o \
BencodeDictionary_t.o \
BencodeDecoder.o \
TorrentFile.o \
Torrent.o \
TorrentCtx.o \
ConnectionHandler.o \
Piece.o \
FileHandler.o

#TorrentPiece_t.o

BIN = bt_client

all: $(BIN)

$(BIN): $(OBJ)	
	$(CC) $(CPFLAGS)  -o $(BIN) $(OBJ) $(LDFLAGS)

%.o:%.c
	$(CC) -c $(CPFLAGS) -o $@ $<  

%.o:%.cpp
	$(CC) -c $(CPFLAGS) -o $@ $<  

run:
	./bt_client -v -b 10.0.0.113 -p 10.0.0.88:6767 -p 10.0.0.2:6767 -s . -l LOG.log  sample/download.mp3.torrent

clean:
	rm -rf $(OBJ) $(BIN) bt_client.tar

tar:
	tar -cvf bt_client.tar LOG.log  Makefile  README  ROADMAP.txt  sample  src

test:
	./bt_client -v -b 10.0.0.217 -p 10.0.0.113:6667 -s . -l LOG.log  sample/download.mp3.torrent

