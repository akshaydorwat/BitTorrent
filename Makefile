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
FileHandler.o \
RequestProcessor.o\
PieceRequestor.o \
PieceProcessor.o


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
	./bt_client -v -b 127.0.0.1 -p 127.0.0.1:6667 -s . -l LOG.log  sample/download.mp3.torrent

test1:
	./bt_client -v -b 127.0.0.1 -p 10.0.0.113:6667  -s . -l LOG.log  sample/download.mp3.torrent

clean:
	rm -rf $(OBJ) $(BIN) bt_client.tar LOG.log

tar:
	tar -cvf bt_client.tar LOG.log  Makefile  README  ROADMAP.txt  sample  src

test2:
	./bt_client -v -b 10.0.0.44 -p 10.0.0.113:6667 -s ./sample/ -l LOG.log  sample/download.mp3.torrent

test4:
	./bt_client -v -b 10.0.0.113 -p 127.0.0.1:6667  -s . -l LOG.log  sample/download.mp3.torrent

run-c1:
	rm -rf dl-c1/*
	./bt_client -v -b 127.0.0.1 -p 127.0.0.1:6667 -p 127.0.0.1:6668 -p 127.0.0.1:6669 -s dl-c1/ -l LOG-c3.log  sample/download.mp3.torrent

run-c2:
	rm -rf dl-c2/*
	./bt_client -v -b 127.0.0.1 -p 127.0.0.1:6667 -p 127.0.0.1:6668 -p 127.0.0.1:6669 -s dl-c2/ -l LOG-c2.log  sample/download.mp3.torrent

run-c3:
	rm -rf dl-c3/*
	./bt_client -v -b 127.0.0.1 -p 127.0.0.1:6667 -p 127.0.0.1:6668 -p 127.0.0.1:6669 -s dl-c3/ -l LOG-c1.log  sample/download.mp3.torrent

run-s1:
	./bt_client -v -b 127.0.0.1 -p 127.0.0.1:6670 -p 127.0.0.1:6671 -p 127.0.0.1:6672 -s sample/ -l LOG-s1.log  sample/download.mp3.torrent

run-s2:
	./bt_client -v -b 127.0.0.1 -p 127.0.0.1:6670 -p 127.0.0.1:6671 -p 127.0.0.1:6672 -s sample/ -l LOG-s2.log  sample/download.mp3.torrent

run-s3:
	./bt_client -v -b 127.0.0.1 -p 127.0.0.1:6670 -p 127.0.0.1:6671 -p 127.0.0.1:6672 -s sample/ -l LOG-s3.log  sample/download.mp3.torrent

