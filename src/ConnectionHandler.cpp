#include "ConnectionHandler.hpp"
#include "Logger.hpp"
#include <unistd.h>

using namespace std;

void ConnectionHandler::handle(string msg){
  LOG(INFO, "Recieved msg : " + msg);
}

void ConnectionHandler::closeConn(){
  close(sfd);
  LOG(INFO, "Closing connection: " + to_string(sfd));
  
}
