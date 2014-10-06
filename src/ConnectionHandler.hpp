#ifndef CONNECTIONHANDLER_HPP
#define CONNECTIONHANDLER_HPP

#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netdb.h>
#include <time.h>
#include "Peer.hpp"
#include "bt_lib.h"

using namespace std;

class ConnectionHandler {

public:
  ConnectionHandler(int fd, struct sockaddr_in src_addr){
    sfd = fd;
    addr = src_addr;
  }

  ConnectionHandler(Peer *p_p, struct sockaddr_in p_sockaddr){
    p = p_p;
    addr = p_sockaddr;
    //time(&last_connected);
    sfd = -1;
  }
  
  // Handle event on connection
  void handle(string msg);

  // Write data to socket descriptor
  void writeConn(char *buff, int buf_len);

  // close connection
  void closeConn();

  // try to reconnect connection
  bool tryConnect();
  
  // send i am live ping 
  void sendLivePing();
  
  // set Peer 
  void setPeer(Peer *peer){
    p = peer;
  }
  
  // get Peer
  Peer* getPeer(){
    return p;
  }
  
  // register socket with rector event register
  void resgiterSocket();

private:  
  Peer *p;
  struct sockaddr_in addr;
  time_t last_connected;
  int sfd;
  char buffer[MAX_PACKET_SIZE];

};

#endif
