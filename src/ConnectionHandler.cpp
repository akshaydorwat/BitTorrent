#include "ConnectionHandler.hpp"
#include "Logger.hpp"
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <math.h>

using namespace std;

void ConnectionHandler::handle(string msg){
  LOG(INFO, "Recieved msg : " + msg);
}

void ConnectionHandler::closeConn(){
  close(sfd);
  LOG(INFO, "Closing connection: " + to_string(sfd));
  
}

bool ConnectionHandler::tryConnect(){
  int i=5;
  int ret;
  int on = 1;
  int sleep;

  LOG(INFO,"Trying to connect Desctination");
  // create socket
  if((sfd = socket(addr.sin_family, SOCK_STREAM, 0)) == -1){
    LOG(ERROR, " Can not initialize socket ");
    exit(EXIT_FAILURE);
  }
  LOG(DEBUG, "socked intiated sucessfully");
  // make socket NON-BLOCKING
  if(ioctl(sfd, FIONBIO, (char *)&on) < 0){
    LOG(ERROR, "Failed to set client socket NON-BLOCKING");
    close(sfd);
    exit(EXIT_FAILURE);
  }

  // try connecting to destination address
  while(i>0){
    
    if((ret = connect(sfd, (struct sockaddr *)&addr, sizeof(addr))) == -1){
      LOG(WARNING,"TRY:"+to_string(5-i)+" Can not connect socket ");
      goto sleep;
    }
    LOG(INFO,"Connected Successfully");
    return true;
  sleep:
    sleep = (int)pow((float)(6-i),(float)2)*1000;
    LOG(DEBUG, "Sleeping for :"+to_string(sleep));
    std::this_thread::sleep_for(std::chrono::milliseconds(sleep));
    i--;
  }
  return false;
}
