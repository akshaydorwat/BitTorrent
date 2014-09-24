/*==============================
   Author : Akshay Dorwat
   Email ID: adorwat@indiana.edu
   created on: 09/22/2014
=================================*/

#include "Peer.hpp"
#include "Logger.hpp"

#include "string"

using namespace std;

void Peer::readMessage(string msg){
  LOG(INFO, "Recieved msg : " + msg);
}
