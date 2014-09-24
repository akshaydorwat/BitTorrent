#include "Peer.hpp"
#include "Logger.hpp"

#include "string"

using namespace std;

void Peer::readMessage(string msg){
  LOG(INFO, "Recieved msg : " + msg);
}
