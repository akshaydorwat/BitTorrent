/*==============================
   Author : Akshay Dorwat
   Email ID: adorwat@indiana.edu
   created on: 09/22/2014
=================================*/
#ifndef PEER_HPP
#define PEER_HPP

#include <time.h>
#include <string>

using namespace std;

class Peer{
  
public:
  Peer(){
    is_initiated_by_me = false;
    is_active = false;
    chocked = true;
    interested = false;
  }

  int sfd;
  time_t last_connect;
  
  void readMessage(string msg);

  /* void setTorrentctx(TorrentCtx* context){
    ctx = context;
    }*/

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
    return chocked;
  }

  void setChocked(bool chock){
    chocked = chock;
  }

  string getMyId(){
    return myId;
  }

private:

  //  TorrentCtx* ctx;
  bool is_initiated_by_me;
  bool is_active;
  string myId;
  string ip;
  string port;
  bool chocked;
  bool interestesd;
};
#endif


