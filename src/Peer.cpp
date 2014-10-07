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
  string msg;
  unsigned int msgType = BT_BITFILED;
  int length = 1 + size;
  ConnectionHandler* c = (ConnectionHandler*)connection;
  
  //Actual payload
  msg.append((const char*)&length, sizeof(int));
  msg.append((const char*)&msgType, sizeof(unsigned int));
  msg.append((const char*)bitVector, size);
  
  LOG(DEBUG,"Bitfield Message is " + msg + " | Message length is " + to_string( msg.size()));
  c->writeConn(msg.data(),msg.length());
}

void Peer::sendUnChoked(){
  string msg;
  int length = 1;
  unsigned int msgType = BT_UNCHOKE;
  ConnectionHandler* c = (ConnectionHandler*)connection;

  msg.append((const char*)&length, sizeof(int));
  msg.append((const char*)&msgType, sizeof(unsigned int));
 
  LOG(DEBUG,"UNchoked Message is " + msg + " | Message length is " + to_string( msg.size()));
  c->writeConn(msg.data(),msg.length());
}


void Peer::sendInterested(){
  string msg;
  int length = 1;
  unsigned int msgType = BT_INTERSTED; 
  ConnectionHandler* c = (ConnectionHandler*)connection;

  msg.append((const char*)&length, sizeof(int));
  msg.append((const char*)&msgType, sizeof(unsigned int));
 
  LOG(DEBUG,"Interested Message is " + msg + " | Message length is " + to_string( msg.size()));
  c->writeConn(msg.data(),msg.length());
}


void Peer::sendHave(int piece){
  string msg;
  int length = 5;
  unsigned int msgType = BT_HAVE; 
  ConnectionHandler* c = (ConnectionHandler*)connection;

  msg.append((const char*)&length, sizeof(int));
  msg.append((const char*)&msgType, sizeof(unsigned int));
  msg.append((const char*)&piece, sizeof(int));

  LOG(DEBUG,"Have Message is " + msg + " | Message length is " + to_string( msg.size()));
  c->writeConn(msg.data(),msg.length());
}

void Peer::sendRequest(int index, int begin, int len){
  string msg;
  int length = 13;
  unsigned int msgType = BT_REQUEST; 
  ConnectionHandler* c = (ConnectionHandler*)connection;

  msg.append((const char*)&length, sizeof(int));
  msg.append((const char*)&msgType, sizeof(unsigned int));
  msg.append((const char*)&index, sizeof(int));
  msg.append((const char*)&begin, sizeof(int));
  msg.append((const char*)&len, sizeof(int));
    
  LOG(DEBUG,"Request Message is " + msg + " | Message length is " + to_string( msg.size()));
  c->writeConn(msg.data(),msg.length());
}

void Peer::sendPiece(int index, int begin, char *block, size_t size){

  string msg;
  unsigned int msgType = BT_PIECE;
  int length = 9 + size;
  ConnectionHandler* c = (ConnectionHandler*)connection;
  

  msg.append((const char*)&length, sizeof(int));
  msg.append((const char*)&msgType, sizeof(unsigned int));
  msg.append((const char*)&index, sizeof(int));
  msg.append((const char*)&begin, sizeof(int));
  msg.append((const char*)block, size);
  
  LOG(DEBUG,"Piece Message Len is  " + to_string( msg.size()));

  c->writeConn(msg.data(),msg.length());

}


