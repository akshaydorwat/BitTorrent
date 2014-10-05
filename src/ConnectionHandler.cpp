#include "ConnectionHandler.hpp"
#include "Logger.hpp"

using namespace std;

void ConnectionHandler::handle(string msg){
  LOG(INFO, "Recieved msg : " + msg);
}
