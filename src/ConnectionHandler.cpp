#include "ConnectionHandler.hpp"
#include "Reactor.hpp"
#include "Logger.hpp"
#include "bt_lib.h"
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <math.h>
#include <string.h>
#include <stdint.h>


#define MAX_TRY 6

using namespace std;

void ConnectionHandler::handle(string msg){
  //LOG(INFO, "Recieved msg : " + msg);

  uint8_t *size;
  int *length;
  const char *message = msg.c_str();
  
  size = (uint8_t*)message;
  if(*size == strlen(PROTOCOL)){
    //if(verifyHandshake(message)){
    //}
  }else if(*length == 0){
    return;
  }else{
    p->readMessage(msg);
  }
}

void ConnectionHandler::closeConn(){
  close(sfd);
  LOG(INFO, "Closing connection: " + to_string(sfd));
}

bool ConnectionHandler::tryConnect(){
  int i = MAX_TRY - 1;
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
    // resiter socket with reactor
    resgiterSocket();
    // Note down latest connected time
    time(&last_connected);

    LOG(INFO,"Connected Successfully");
    return true;
  sleep:
    sleep = (int)pow((float)(MAX_TRY-i),(float)2)*1000;
    LOG(DEBUG, "Sleeping for :"+to_string(sleep));
    std::this_thread::sleep_for(std::chrono::milliseconds(sleep));
    i--;
  }
  return false;
}

void ConnectionHandler::resgiterSocket(){
  Reactor::getInstance()->registerEvent(sfd,this);
}

void ConnectionHandler::writeConn(char *buff, int buf_len){
  memcpy(buffer, buff, buf_len);
  write(sfd, buffer, buf_len);
}
