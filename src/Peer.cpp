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
  if(connection ){
    LOG(DEBUG, "Createing new Connection handler for peer");
    ConnectionHandler *conn = new ConnectionHandler(this, getSocketAddr());
    connection = conn;
  }

  // try connecting 
  ConnectionHandler* c = (ConnectionHandler*)connection;
  if(!active){
    if(!c->tryConnect()){
      return;
    }
  }
  // connection is active now
  active = true;
  // store handler for future use
  
  // connection is initiated by me
  initiated_by_me = true;
    
  // Now send handshake
  
}

void Peer::sendHandshake(){
  bt_handshake_t handshake;
  ConnectionHandler* c = (ConnectionHandler*)connection;
  
  handshake.len = (uint8_t)strlen(PROTOCOL);
  memcpy(&handshake.protocol, PROTOCOL, sizeof(handshake.protocol));
  bzero(&handshake.reserve, sizeof(handshake.reserve));
  memcpy(&handshake.infoHash, ctx->getInfoHash().c_str(), sizeof(handshake.infoHash));
  memcpy(&handshake.peerId, getId(), sizeof(handshake.peerId));
  c->writeConn((char*)&handshake, sizeof(handshake));
}


