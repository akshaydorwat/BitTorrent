/*==============================
   Author : Akshay Dorwat
   Email ID: adorwat@indiana.edu
   created on: 09/22/2014
=================================*/
#ifndef PEER_HPP
#define PEER_HPP

#include <time.h>
#include <string>
#include "TorrentCtx.hpp"
#include "bt_lib.h"

using namespace std;

class Peer{
  
public:
  Peer(TorrentCtx* p_ctx, peer_t *p_p){
    ctx = p_ctx;
    p = p_p;
    is_initiated_by_me = false;
    is_active = false;
  }

  int sfd;
  time_t last_connect;
  
  void readMessage(string msg);

  // Start connection to other Peers in the swarn
  void startConnection();

  void setTorrentctx(TorrentCtx* context){
    ctx = context;
  }

  bool isActive(){
    return is_active;
  }
  
  void setActive(bool active){
    is_active = active;
  }

  bool isInitiatedByMe(){
    return is_initiated_by_me;
  }
  
  void setInitiatedByMe(bool by_me){
    is_initiated_by_me = by_me;
  }
  
  bool isChocked(){
    return (p->choked == 1);
  }

  void setChocked(bool chock){
    p->choked = (chock ? 1 : 0);
  }

  unsigned char* getId(){
    return p->id;
  }

  unsigned short getPort(){
    return p->port;
  }
  
  struct sockaddr_in getSocketAddr(){
    return p->sockaddr;
  }
  
private:

  TorrentCtx* ctx;
  peer_t *p;
  void *connection;
  bool is_initiated_by_me;
  bool is_active;
};
#endif


