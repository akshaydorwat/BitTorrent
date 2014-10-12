/*==============================
   Author : Akshay Dorwat
   Email ID: adorwat@indiana.edu
   created on: 09/22/2014
=================================*/

#include "Peer.hpp"
#include "Logger.hpp"
#include "ConnectionHandler.hpp"
#include "string"
#include "bt_lib.h"
#include <stdint.h>

using namespace std;

void Peer::readMessage(const char *msg, size_t len){

  uint8_t msgType;
  int runner = 0;
    
  memcpy((void*)&msgType,(const void *)(msg+runner), sizeof(uint8_t));
  runner = runner + sizeof(uint8_t);

  printf("Message Type is %u & message len : %d\n",msgType, (int)len);

  switch(msgType){
    
  case BT_CHOKE:
    if(len == 1){
      LOG(INFO,"Recieved CHOKE message");
      setChocked(true);
    }
    break;

  case BT_UNCHOKE: 
    if(len == 1){
      LOG(INFO, "Recieved UNCHOKE message");
      setChocked(false);
    }
    break;

  case BT_INTERSTED :
    if(len == 1){
      LOG(INFO, "Recieved INTERESTED message");
      setInterested(true);
    }
    break;

  case BT_NOT_INTERESTED :
    if(len == 1){
      LOG(INFO, "Recieved NOT INTERESTED message");
      setInterested(false);
    }
    break;

  case BT_HAVE :
    if(len == 5){
      LOG(INFO, "Recieved HAVE message");
    }
    break;

  case BT_BITFILED :
    if(len > 1){
      LOG(INFO, "Recieved BTFILED message");
      size_t sizeOfBitField = len - 1;
      if(sizeOfBitField != ctx->getBitVectorSize()){
	LOG(ERROR, "Bit vector size didnt match");
	exit(EXIT_FAILURE);
      }
      LOG(DEBUG,"Bit Vector size matched copying it into local peer");
      copyBitVector((char *)(msg+runner) , ctx->getNumOfPieces());
    }
    break;
    
  case BT_REQUEST :
    if(len == 13){
      int index;
      int begin;
      int length;
      
      memcpy((void*)&index, (const void *)(msg+runner), sizeof(int));
      runner = runner + sizeof(int);
      
      memcpy((void*)&begin, (const void *)(msg+runner), sizeof(int));
      runner = runner + sizeof(int);
      
      memcpy((void*)&length, (const void *)(msg+runner), sizeof(int));
      runner = runner + sizeof(int);

      LOG(INFO,"Received REQUEST message : index :"+to_string(index) + " Begin :"+to_string(begin)+ "len :" + to_string (length));
      
      // queue this request to Torrent context request threadpool
    }
    break;
    
  case BT_PIECE :
    if(len > 9){
      int index;
      int begin;
      string block;
      int blockLen = len - 9;

      memcpy((void*)&index, (const void *)(msg+runner), sizeof(int));
      runner = runner + sizeof(int);
      
      memcpy((void*)&begin, (const void *)(msg+runner), sizeof(int));
      runner = runner + sizeof(int);
      
      block = string((const char *)(msg+runner), (size_t)blockLen);

      LOG(INFO,"Received PIECE message : index :"+to_string(index) + " Begin :"+to_string(begin) + "Data : " + block);
      // queue this request to Torrent context piece threadpool
    }
    break;
    
  case BT_CANCEL :
    LOG(INFO, "Recieved CANCEL message");
    break;

  }
}

void Peer::startConnection(){
  
  // Create connection Handler for peer
  if(!isConnectionEstablished()){
    LOG(DEBUG, "Createing new Connection handler for peer");
    ConnectionHandler *conn = new ConnectionHandler(this, getSocketAddr(), ctx);
    // store handler for future use
    connection = conn;
  }
  // try connecting 
  ConnectionHandler* c = (ConnectionHandler*)connection;
  if(!c->tryConnect()){
      return;
  }
  // connection is initiated by me
  initiated_by_me = true;
  // Now send handshake
  c->sendHandshake();
}

//TODO: If connection is closed or dropped but packet are in qeuue i can try to reconnect. Considering it was glith in the network. Need to think through

void Peer::newConnectionMade(){
  char test[11] = "0123456789";
  sendUnChoked();
  sendBitField((const char *)ctx->getPiecesBitVector(), ctx->getBitVectorSize());

  //sendInterested();
  //sendHave(50);
  //sendRequest(500, 500, 500);
  //sendPiece(0,0,test,11);
}

void Peer::setBitVector(int piece){
  m_lock.lock();
  bitVector[piece] = true;
  m_lock.unlock();
}

void Peer::copyBitVector(char *piecesBitVector, int numOfPieces){
  int i;

  // Reset the size of bit vector
  bitVector.resize(numOfPieces, false);
  
  for(i=0; i<numOfPieces; i++){
    size_t mbyte, mbit;

    mbyte = i / 8;
    mbit = i % 8;
    
    if(((piecesBitVector[mbyte] << mbit) & 0x80 ) >> 7){
      LOG(DEBUG, "BIT[" + to_string(i) + "] = true");
      setBitVector(i);
    }else{
      LOG(DEBUG, "BIT[" + to_string(i) + "] = false");
    }
  }
}

void Peer::sendLiveMessage(){
  int length = 0;
  ConnectionHandler* c = (ConnectionHandler*)connection;
  int buff_size = length+sizeof(int);
  char buff[buff_size];

  memcpy((void*)buff, (const void*)&length, sizeof(int));

  LOG(DEBUG,"Sending Live Message");
  c->writeConn(buff, buff_size);
}

void Peer::sendBitField(const char *bitVector, size_t size){
  
  uint8_t msgType = BT_BITFILED;
  int length = 1 + size;
  ConnectionHandler* c = (ConnectionHandler*)connection;
  int buff_size = length+sizeof(int);
  char buff[buff_size];
  char *runner = (char*)buff;
  
  memcpy((void*)runner, (const void*)&length, sizeof(int));
  runner = runner + sizeof(int);

  memcpy((void*)runner,(const void*)&msgType, sizeof(uint8_t));
  runner = runner + sizeof(uint8_t);

  memcpy((void*)runner,(const void*)bitVector, size);

  LOG(DEBUG,"Sending Bitfield Message");
  c->writeConn(buff, buff_size);
}

void Peer::sendUnChoked(){

  int length = 1;
  uint8_t msgType = BT_UNCHOKE;
  ConnectionHandler* c = (ConnectionHandler*)connection;
  int buff_size = length+sizeof(int);
  char buff[buff_size];
  char *runner = (char*)buff;
  
  memcpy((void*)runner, (const void*)&length, sizeof(int));
  runner = runner + sizeof(int);

  memcpy((void*)runner,(const void*)&msgType, sizeof(uint8_t));
  runner = runner + sizeof(uint8_t);

 
  LOG(DEBUG,"Sending Unchoked Message");
  c->writeConn(buff, buff_size);
}


void Peer::sendInterested(){

  int length = 1;
  uint8_t msgType = BT_INTERSTED;
  ConnectionHandler* c = (ConnectionHandler*)connection;
  int buff_size = length+sizeof(int);
  char buff[buff_size];
  char *runner = (char*)buff;
  
  memcpy((void*)runner, (const void*)&length, sizeof(int));
  runner = runner + sizeof(int);

  memcpy((void*)runner,(const void*)&msgType, sizeof(uint8_t));
  runner = runner + sizeof(uint8_t);
 
  LOG(DEBUG,"Sending Interested Message ");
  c->writeConn(buff, buff_size);
}


void Peer::sendHave(int piece){

  int length = 5;
  uint8_t msgType = BT_HAVE; 
  ConnectionHandler* c = (ConnectionHandler*)connection;
  int buff_size = length+sizeof(int);
  char buff[buff_size];
  char *runner = (char*)buff;
  
  memcpy((void*)runner, (const void*)&length, sizeof(int));
  runner = runner + sizeof(int);

  memcpy((void*)runner,(const void*)&msgType, sizeof(uint8_t));
  runner = runner + sizeof(uint8_t);

  memcpy((void*)runner,(const void*)&piece, sizeof(int));

  LOG(DEBUG,"Sendinf Have Message");
  c->writeConn(buff, buff_size);
}

void Peer::sendRequest(int index, int begin, int len){

  int length = 13;
  uint8_t msgType = BT_REQUEST; 
  ConnectionHandler* c = (ConnectionHandler*)connection;
  int buff_size = length+sizeof(int);
  char buff[buff_size];
  char *runner = (char*)buff;
  
  memcpy((void*)runner, (const void*)&length, sizeof(int));
  runner = runner + sizeof(int);

  memcpy((void*)runner,(const void*)&msgType, sizeof(uint8_t));
  runner = runner + sizeof(uint8_t);

  memcpy((void*)runner,(const void*)&index, sizeof(int));
  runner = runner + sizeof(int);

  memcpy((void*)runner,(const void*)&begin, sizeof(int));
  runner = runner + sizeof(int);

  memcpy((void*)runner,(const void*)&len, sizeof(int));
  
  LOG(DEBUG,"Sending Request Message");
  c->writeConn(buff, buff_size);
}

void Peer::sendPiece(int index, int begin, const char *block, size_t size){

  uint8_t msgType = BT_PIECE;
  int length = 9 + size;
  ConnectionHandler* c = (ConnectionHandler*)connection;
  int buff_size = length+sizeof(int);
  char buff[buff_size];
  char *runner = (char*)buff;
  
  memcpy((void*)runner, (const void*)&length, sizeof(int));
  runner = runner + sizeof(int);

  memcpy((void*)runner,(const void*)&msgType, sizeof(uint8_t));
  runner = runner + sizeof(uint8_t);

  memcpy((void*)runner,(const void*)&index, sizeof(int));
  runner = runner + sizeof(int);

  memcpy((void*)runner,(const void*)&begin, sizeof(int));
  runner = runner + sizeof(int);

  memcpy((void*)runner,(const void*)block, size);

  LOG(DEBUG,"Sending Piece Message");
  c->writeConn(buff, buff_size);
}


