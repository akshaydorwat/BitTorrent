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
  //sendBitField();
  //sendUnChoked();
}

/*void Peer::sendBitField(){
  
}

void Peer::sendInterested(){
}

void Peer::sendUnChoked(){
}

void Peer::sendHave(){
}

void Peer::sendRequest(){
}

void Peer::sendPiece(){
}*/


