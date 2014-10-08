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

using namespace std;

void Peer::readMessage(string msg){
  LOG(INFO, "Recieved msg : " + msg);
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
  LOG(INFO,"Send bitfield and Unchoke message now");
  sendBitField(ctx->getPiecesBitVector(), ctx->getBitVectorSize());
  sendUnChoked();
}

void Peer::sendBitField(char *bitVector, size_t size){
  
  unsigned int msgType = BT_BITFILED;
  int length = 1 + size;
  ConnectionHandler* c = (ConnectionHandler*)connection;
  int buff_size = length+sizeof(int);
  char buff[buff_size];
  char *runner = (char*)buff;
  
  memcpy((void*)runner, (const void*)&length, sizeof(int));
  runner = runner + sizeof(int);

  memcpy((void*)runner,(const void*)&msgType, sizeof(unsigned int));
  runner = runner + sizeof(unsigned int);

  memcpy((void*)runner,(const void*)&bitVector, size);

  LOG(DEBUG,"Sending Bitfield Message");
  c->writeConn(buff, buff_size);
}

void Peer::sendUnChoked(){

  int length = 1;
  unsigned int msgType = BT_UNCHOKE;
  ConnectionHandler* c = (ConnectionHandler*)connection;
  int buff_size = length+sizeof(int);
  char buff[buff_size];
  
  memcpy((void*)buff, (const void*)&length, sizeof(int));
  memcpy((void*)(buff + sizeof(int)),(const void*)&msgType, sizeof(unsigned int));
 
  LOG(DEBUG,"Sending Unchoked Message");
  c->writeConn(buff, buff_size);
}


void Peer::sendInterested(){

  int length = 1;
  unsigned int msgType = BT_INTERSTED;
  ConnectionHandler* c = (ConnectionHandler*)connection;
  int buff_size = length+sizeof(int);
  char buff[buff_size];
  
  memcpy((void*)buff, (const void*)&length, sizeof(int));
  memcpy((void*)(buff + sizeof(int)),(const void*)&msgType, sizeof(unsigned int));
 
  LOG(DEBUG,"Sending Interested Message ");
  c->writeConn(buff, buff_size);
}


void Peer::sendHave(int piece){

  int length = 5;
  unsigned int msgType = BT_HAVE; 
  ConnectionHandler* c = (ConnectionHandler*)connection;
  int buff_size = length+sizeof(int);
  char buff[buff_size];
  char *runner = (char*)buff;
  
  memcpy((void*)runner, (const void*)&length, sizeof(int));
  runner = runner + sizeof(int);

  memcpy((void*)runner,(const void*)&msgType, sizeof(unsigned int));
  runner = runner + sizeof(unsigned int);

  memcpy((void*)runner,(const void*)&piece, sizeof(int));

  LOG(DEBUG,"Sendinf Have Message");
  c->writeConn(buff, buff_size);
}

void Peer::sendRequest(int index, int begin, int len){

  int length = 13;
  unsigned int msgType = BT_REQUEST; 
  ConnectionHandler* c = (ConnectionHandler*)connection;
  int buff_size = length+sizeof(int);
  char buff[buff_size];
  char *runner = (char*)buff;
  
  memcpy((void*)runner, (const void*)&length, sizeof(int));
  runner = runner + sizeof(int);

  memcpy((void*)runner,(const void*)&msgType, sizeof(unsigned int));
  runner = runner + sizeof(unsigned int);

  memcpy((void*)runner,(const void*)&index, sizeof(int));
  runner = runner + sizeof(int);

  memcpy((void*)runner,(const void*)&begin, sizeof(int));
  runner = runner + sizeof(int);

  memcpy((void*)runner,(const void*)&len, sizeof(int));
  
  LOG(DEBUG,"Sending Request Message");
  c->writeConn(buff, buff_size);
}

void Peer::sendPiece(int index, int begin, char *block, size_t size){

  unsigned int msgType = BT_PIECE;
  int length = 9 + size;
  ConnectionHandler* c = (ConnectionHandler*)connection;
  int buff_size = length+sizeof(int);
  char buff[buff_size];
  char *runner = (char*)buff;
  
  memcpy((void*)runner, (const void*)&length, sizeof(int));
  runner = runner + sizeof(int);

  memcpy((void*)runner,(const void*)&msgType, sizeof(unsigned int));
  runner = runner + sizeof(unsigned int);

  memcpy((void*)runner,(const void*)&index, sizeof(int));
  runner = runner + sizeof(int);

  memcpy((void*)runner,(const void*)&begin, sizeof(int));
  runner = runner + sizeof(int);

  memcpy((void*)runner,(const void*)&block, size);

  LOG(DEBUG,"Sending Piece Message");
  c->writeConn(buff, buff_size);

}


