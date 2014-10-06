/*==============================
   Author : Akshay Dorwat
   Email ID: adorwat@indiana.edu
   created on: 09/22/2014
=================================*/

#include "Peer.hpp"
#include "Logger.hpp"
#include "ConnectionHandler.hpp"
#include "string"

using namespace std;

void Peer::readMessage(string msg){
  LOG(INFO, "Recieved msg : " + msg);
}

void Peer::startConnection(){
  if(!isActive()){
    LOG(DEBUG, "Createing new Connection handler for peer");
    ConnectionHandler *conn = new ConnectionHandler(this, getSocketAddr());
    conn->tryConnect();
  }
}
