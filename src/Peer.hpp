/*==============================
   Author : Akshay Dorwat
   Email ID: adorwat@indiana.edu
   created on: 09/22/2014
=================================*/
#ifndef PEER_HPP
#define PEER_HPP

#include <mutex>
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
    initiated_by_me = false;
    connection = NULL;
    setChocked(true);
    setInterested(false);
    //active = false;
  }
  
  ~Peer();

  int sfd;
  time_t last_connect;
  
  // read torrent protocol message
  void readMessage(const char *msg, size_t len);

  // Start connection to other Peers in the swarn
  void startConnection();


  void setTorrentctx(TorrentCtx* context){
    ctx = context;
  }

  /*  bool isActive(){
    return active;
  }
  
  void setActive(bool act){
    active = act;
  }
  */
  bool isInitiatedByMe(){
    return initiated_by_me;
  }
  
  void setInitiatedByMe(bool by_me){
    initiated_by_me = by_me;
  }
  
  bool isChocked(){
    return (p->choked == 1);
  }

  void setChocked(bool chock){
    p->choked = (chock ? 1 : 0);
  }

  bool isInterested(){
    return (p->interested == 1);
  }

  void setInterested(bool interested){
    p->interested = (interested ? 1 : 0);
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
  
  bool isConnectionEstablished(){
    return ((connection == NULL) ? false : true);
  }
  
  void destroyConnection(){
    connection = NULL;
    setChocked(true);
    setInterested(false);
  }

  void setConnection(void *c){
    connection = c;
  }

  //stuff to do when new connection is made
  void newConnectionMade();

  // send bit field message
  void sendBitField(const char *bitVector, size_t size);

  //send unchoked message
  void sendUnChoked();

  // send interested message
  void sendInterested();

  // send have message
  void sendHave(int piece);
    
  // send request
  void sendRequest(int index, int begin, int len);
  
  // send piece
  void sendPiece(int index, int begin, const char *block, size_t size);

  //send live message
  void sendLiveMessage();

  //set bit vector
  void setBitVector(int piece);

  //get bit from vector
  bool getBitVector(int piece);
  //copy bit vector
  void copyBitVector(char *piecesBitVector, int numOfPieces);

private:

  TorrentCtx* ctx;
  peer_t *p;
  void *connection;
  bool initiated_by_me;
  vector<bool> bitVector;
  mutex m_lock;
  //bool active;

};
#endif


