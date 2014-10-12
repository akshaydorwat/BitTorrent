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

void Peer::readMessage(string msg){

  //LOG(INFO, "Recieved msg : " + msg );
  int length;
  const char *payload = msg.data();
  int payloadLen = msg.size();
  int runner = 0;
  
  do{
    // length of the message in the header    
    memcpy((void*)&length,(void *)(payload+runner), sizeof(length));
    runner = runner + sizeof(length);

    if(length == 0){
      LOG(INFO, "Reciecved Live message");
    }else{
      ctx->processMsg((const char *)(payload + runner), length, (void *)this);
    }
    runner = runner + length;

  }while(runner < payloadLen);
    
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


