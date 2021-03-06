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
#include <mutex>

using namespace std;

class ConnectionHandler {

public:
  ConnectionHandler(int fd, struct sockaddr_in src_addr, void *ctx){
    sfd = fd;
    addr = src_addr;
    torrentCtx = ctx;
    handshakeComplete = false;
    peerConnected = false;
    p = NULL;
  }

  ConnectionHandler(Peer *p_p, struct sockaddr_in p_sockaddr, void *ctx){
    p = p_p;
    addr = p_sockaddr;
    //time(&last_connected);
    sfd = -1;
    torrentCtx = ctx;
    handshakeComplete = false;
    peerConnected = true;
  }
  
  // Handle event on connection
  void handle(string msg);

  // Send hand shake
  void sendHandshake();

  // Verify handshake
  bool verifyHandshake(const char *message);

  // Check for live message
  bool checkForLive(const char *message);

  // check for hanshake message
  bool checkForhandshakeMsg(const char *message);

  // Write data to socket descriptor
  void writeConn(const char *buff, int buf_len);

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

  //set peer connected
  void setPeerConnected( bool val){
    peerConnected = val;
  }

private:  

  Peer *p;
  mutex m_lock;
  void *torrentCtx;
  bool handshakeComplete;
  bool peerConnected;
  struct sockaddr_in addr;
  time_t last_connected;
  int sfd;
  //char buffer[MAX_PACKET_SIZE];
  string buffer;
};

#endif
