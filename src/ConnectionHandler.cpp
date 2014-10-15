#include "ConnectionHandler.hpp"
#include "Reactor.hpp"
#include "Logger.hpp"
#include "Peer.hpp"
#include "bt_lib.h"
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <math.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

#define MAX_TRY 6

using namespace std;

void ConnectionHandler::handle(string msg){
  int runner = 0;
  buffer.append(msg);
  const char *message;
  int msgLen = buffer.length();
  int length; //bt protocol length
  

  while(msgLen > 0){
    runner = 0;
    message = buffer.data();
    msgLen = buffer.length();

    if(checkForhandshakeMsg((const char*)(message))){
      if(msgLen < ((int)sizeof(bt_handshake_t))){
	return;
      }
      // Verify the hanshake
      if(verifyHandshake(message)){
	if(!p->isInitiatedByMe()){
	  sendHandshake();
	  p->newConnectionMade();
	}
	handshakeComplete = true;
	//runner = runner+sizeof(bt_handshake_t);
	string temp = buffer.substr(sizeof(bt_handshake_t),buffer.length());
	buffer.clear();
	buffer.append(temp);
	continue;
      }else if(!handshakeComplete){
	closeConn();
	delete this;
	return;
      }
    }
    
    length = 0;
    memcpy((void*)&length,(void *)(message), sizeof(length));
    runner = runner + sizeof(length);


    /*if(length == 0){
      LOG(INFO, "Reciecved Live message");
      runner = runner + length;
      continue;
      }*/
    
    if(msgLen < (runner+length) ){
      //LOG(DEBUG, "BT Packet length is " + to_string(length));
      LOG(DEBUG, "Waiting on full message EXpected : " + to_string(runner+length) + "actaul :" + to_string(msgLen));
      return;
    }
    
    // Send mesage to Peer for further investigation
    if(p && handshakeComplete){
      LOG(DEBUG, "Seding message to peer BT Packet length is " + to_string(length));
      //LOG(DEBUG, "Sending message to peer for handling");
      p->readMessage((const char*)(message+runner), (size_t)length);
      string temp = buffer.substr(runner + length, buffer.length());
      buffer.clear();
      buffer.append(temp);
      continue;
    }
    
    closeConn();
    delete this;

  }
  
  /*if((runner) == msgLen){
    LOG(DEBUG, "Clearing buffer");
    buffer.clear();
  }else{
    LOG(DEBUG, "#############Can not clear buffer Runner " + to_string(runner) + " and msgLen" );
    }*/
}


bool ConnectionHandler::verifyHandshake(const char *message){
  bt_handshake_t *handshake = (bt_handshake_t*)message;
  TorrentCtx *ctx = (TorrentCtx*)torrentCtx;
  //compare protocol lengh
  if(handshake->len != strlen(PROTOCOL)){
    LOG(DEBUG, "Protocol length didnt match");
    return false;
  }
  // comapre protocol name
  if(memcmp(handshake->protocol,PROTOCOL, strlen(PROTOCOL)) != 0){
    LOG(DEBUG, "Protocol name didnt match");
    return false;
  }
  // comapre info hash
  if(memcmp(handshake->infoHash,ctx->getInfoHash().c_str(), 20) != 0){
    LOG(DEBUG, "info hash didnt match");
    return false;
  }
  // Try to associate connection with peer
  p = (Peer*)ctx->getPeer((unsigned char*)handshake->peerId);
  if(p == NULL){
    LOG(WARNING, "Peer rejected ");
    // close connection
    // closeConn();
    return false;
  }else{
    // store pointer to peer in the connection
    if(!p->isConnectionEstablished()){
      LOG(INFO,"New Peer joined");
      p->setConnection((void*)this);
    }
  }
  return true;
}

bool ConnectionHandler::checkForhandshakeMsg(const char *message){
  bt_handshake_t *handshake = (bt_handshake_t*)message;
  if(handshake->len == strlen(PROTOCOL)){
    LOG(DEBUG, "Recieved Handshake Message");
    return true;
  }
  return false;
}

bool ConnectionHandler::checkForLive(const char *message){
  bt_msg_t *msg = (bt_msg_t*)message;
  
  if(msg->length != 0){
    return false;
  }
  return true;
}

void ConnectionHandler::sendHandshake(){
  bt_handshake_t handshake;
  TorrentCtx *ctx = (TorrentCtx*)torrentCtx;

  handshake.len = (uint8_t)strlen(PROTOCOL);
  memcpy(&handshake.protocol, PROTOCOL, sizeof(handshake.protocol));
  bzero(&handshake.reserve, sizeof(handshake.reserve));
  memcpy(&handshake.infoHash, ctx->getInfoHash().c_str(), sizeof(handshake.infoHash));
  memcpy(&handshake.peerId, ctx->getPeerId(), sizeof(handshake.peerId));
  writeConn((char*)&handshake, sizeof(handshake));
}


void ConnectionHandler::closeConn(){
  close(sfd);
  // unregister socket from reactor if we want to do sucide
  Reactor::getInstance()->unRegisterEvent(sfd);
  LOG(INFO, "Closing connection: " + to_string(sfd));
  if(p){
    p->setChocked(true);
    p->setInterested(false);
    p->destroyConnection();
  }
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

void ConnectionHandler::writeConn(const char *buff, int buf_len){
  int result;

  //m_lock.lock();
  //memcpy(buffer, buff, buf_len);
  result =   write(sfd, buff, buf_len);
  if (result == -1) {
    if (errno == EWOULDBLOCK){
      return;
    }
    LOG(ERROR, "Could not write to socket");
  }
  //m_lock.unlock();
  LOG(DEBUG, "Number of bytes written : " + to_string(result));
}
