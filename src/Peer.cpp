/*==============================
   Author : Akshay Dorwat
   Email ID: adorwat@indiana.edu
   created on: 09/22/2014
=================================*/

#include "Peer.hpp"
#include "Logger.hpp"
#include "ConnectionHandler.hpp"
#include "Piece.hpp"
#include "string"
#include "bt_lib.h"
#include <chrono>
#include <stdint.h>

using namespace std;

Peer::~Peer(){
  ConnectionHandler *h = (ConnectionHandler*) connection;
  if(isConnectionEstablished()){
    h->closeConn();
    delete h;
  }
  // free the memory allocted by bt_args
  free(p);
}

string Peer::printPeerInfo()
{
	char ip[17], port[6], id[41];
	for (size_t i=0; i<41; i++)
	{
		if (i < 6) port[i] = '\0';
		if (i < 17) ip[i] = '\0';
		if (i < 41) id[i] = '\0';
	}

	getPeerIpPortId(p, ip, port, id);

	string info("Peer ");
	//info.append(id);
	info.append("[");
	info.append(ip);
	info.append(":");
	info.append(port, 5);
	info.append("]");
	
	return info;
}

void Peer::readMessage(const char *msg, size_t len){

  uint8_t msgType;
  int runner = 0;
    
  memcpy((void*)&msgType,(const void *)(msg+runner), sizeof(uint8_t));
  runner = runner + sizeof(uint8_t);

  //printf("Message Type is %u & message len : %d\n",msgType, (int)len);
  //LOG (DEBUG, printPeerInfo() + " read message type " + to_string(msgType) + " of length " + to_string((int) len));

  switch(msgType){
    
  case BT_CHOKE:
    if(len == 1){
      LOG(INFO, "CHOKED by" + printPeerInfo());
      setChocked(true);
    }
    break;

  case BT_UNCHOKE: 
    if(len == 1){
      LOG(INFO, "UNCHOKED by " + printPeerInfo());
      setChocked(false);
    }
    break;

  case BT_INTERSTED :
    if(len == 1){
      LOG(INFO, printPeerInfo() + " is INTERESTED");
      setInterested(true);
      // send Unchoke message
      sendUnChoked();
    }
    break;

  case BT_NOT_INTERESTED :
    if(len == 1){
      LOG(INFO, printPeerInfo() + " is NOT INTERESTED");
      setInterested(false);
    }
    break;

  case BT_HAVE :
    if(len == 5){
      LOG(INFO, printPeerInfo() + " replied HAVE");
    }
    break;

  case BT_BITFILED :
    if(len > 1){
      LOG(INFO, printPeerInfo() + " sent BITFIELD.");
      size_t sizeOfBitField = len - 1;
      if(sizeOfBitField != ctx->getBitVectorSize()){
	LOG(ERROR, "Failed to read BITFIELD from " + printPeerInfo());
	exit(EXIT_FAILURE);
      }
      //LOG(DEBUG,"Bit Vector size matched copying it into local peer");
      copyBitVector((char *)(msg+runner) , ctx->getNumOfPieces());
      if(!ctx->isComplete()){
	setInterested(true);
	sendInterested();
      }
    }
    break;
    
  case BT_REQUEST :
    if(len == 13){
      int index;
      int begin;
      int length;
      
      memcpy((void*)&index, (const void *)(msg+runner), sizeof(int));
      runner = runner + sizeof(int);
      
      memcpy((void*)&begin, (const void *)(msg+runner), sizeof(int));
      runner = runner + sizeof(int);
      
      memcpy((void*)&length, (const void *)(msg+runner), sizeof(int));
      runner = runner + sizeof(int);

      LOG(INFO, printPeerInfo() + " REQUESTING : Piece#"+to_string(index) + " Offset "+to_string(begin)+ "Length " + to_string (length));
      
	lastCommunicationTime = clock();//chrono::high_resolution_clock::now();

      ctx->requestProcessor->addTask(index, begin, length, this);
    }
    break;
    
  case BT_PIECE :
    if(len > 9){
      int index;
      int begin;
      string block;
      int blockLen = len - 9;

      memcpy((void*)&index, (const void *)(msg+runner), sizeof(int));
      runner = runner + sizeof(int);
      
      memcpy((void*)&begin, (const void *)(msg+runner), sizeof(int));
      runner = runner + sizeof(int);
      
      block = string((const char *)(msg+runner), (size_t)blockLen);

      //LOG(INFO,"Received PIECE message : index :"+to_string(index) + " Begin :"+to_string(begin) + "Data : " + block);
      LOG(INFO, printPeerInfo() + " sent PIECE : Piece#"+to_string(index) + " Offset "+to_string(begin));

	taken += (size_t) blockLen;
	//auto elapsed = chrono::high_resolution_clock::now() - lastCommunicationTime;
	totalCommunicationTime += clock() - lastCommunicationTime;//chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
	lastCommunicationTime = clock();//chrono::high_resolution_clock::now();

      ctx->pieceProcessor->addTask(index, begin, block, this);
    }
    break;
    
  case BT_CANCEL :
    LOG(INFO, printPeerInfo() + " sent CANCEL");
    break;

  }
}

void Peer::startConnection(){
  
  // Create connection Handler for peer
  if(!isConnectionEstablished()){
    LOG(DEBUG, printPeerInfo() + " creating new Connection Handler.");
    ConnectionHandler *conn = new ConnectionHandler(this, getSocketAddr(), ctx);
    // store handler for future use
    connection = conn;
  }
  // try connecting 
  ConnectionHandler* c = (ConnectionHandler*)connection;
  if(!c->tryConnect()){
      return;
  }
  // connection is initiated by me
  initiated_by_me = true;
  // Now send handshake
  c->sendHandshake();
}

//TODO: If connection is closed or dropped but packet are in qeuue i can try to reconnect. Considering it was glith in the network. Need to think through

void Peer::newConnectionMade(){
  // send bit field message
  sendBitField((const char *)ctx->getPiecesBitVector(), ctx->getBitVectorSize());
  
}

void Peer::setBitVector(int piece){
  m_lock.lock();
  bitVector[piece] = true;
  m_lock.unlock();
}

bool Peer::getBitVector(int piece){
  return bitVector[piece];
  
}
void Peer::copyBitVector(char *piecesBitVector, int numOfPieces){
  int i;

  // Reset the size of bit vector
  bitVector.resize(numOfPieces, false);
  
  for(i=0; i<numOfPieces; i++){
    size_t mbyte, mbit;

    mbyte = i / 8;
    mbit = i % 8;
    
    if(((piecesBitVector[mbyte] << mbit) & 0x80 ) >> 7){
      //LOG(DEBUG, "BIT[" + to_string(i) + "] = true");
      setBitVector(i);
    }else{
      //LOG(DEBUG, "BIT[" + to_string(i) + "] = false");
    }
  }
}

void Peer::sendLiveMessage(){
  int length = 0;
  ConnectionHandler* c = (ConnectionHandler*)connection;
  int buff_size = length+sizeof(int);
  char buff[buff_size];

  memcpy((void*)buff, (const void*)&length, sizeof(int));

  LOG(DEBUG, "Sending LIVE message to " + printPeerInfo());
  c->writeConn(buff, buff_size);
}

void Peer::sendBitField(const char *bitVector, size_t size){
  
  uint8_t msgType = BT_BITFILED;
  int length = 1 + size;
  ConnectionHandler* c = (ConnectionHandler*)connection;
  int buff_size = length+sizeof(int);
  char buff[buff_size];
  char *runner = (char*)buff;
  
  memcpy((void*)runner, (const void*)&length, sizeof(int));
  runner = runner + sizeof(int);

  memcpy((void*)runner,(const void*)&msgType, sizeof(uint8_t));
  runner = runner + sizeof(uint8_t);

  memcpy((void*)runner,(const void*)bitVector, size);

  LOG(DEBUG, "Sending BITFIELD message to " + printPeerInfo());
  c->writeConn(buff, buff_size);
}

void Peer::sendUnChoked(){

  int length = 1;
  uint8_t msgType = BT_UNCHOKE;
  ConnectionHandler* c = (ConnectionHandler*)connection;
  int buff_size = length+sizeof(int);
  char buff[buff_size];
  char *runner = (char*)buff;
  
  memcpy((void*)runner, (const void*)&length, sizeof(int));
  runner = runner + sizeof(int);

  memcpy((void*)runner,(const void*)&msgType, sizeof(uint8_t));
  runner = runner + sizeof(uint8_t);

 
  LOG(DEBUG, "Sending UNCHOKE message to " + printPeerInfo());
  c->writeConn(buff, buff_size);
}


void Peer::sendInterested(){

  int length = 1;
  uint8_t msgType = BT_INTERSTED;
  ConnectionHandler* c = (ConnectionHandler*)connection;
  int buff_size = length+sizeof(int);
  char buff[buff_size];
  char *runner = (char*)buff;
  
  memcpy((void*)runner, (const void*)&length, sizeof(int));
  runner = runner + sizeof(int);

  memcpy((void*)runner,(const void*)&msgType, sizeof(uint8_t));
  runner = runner + sizeof(uint8_t);
 
  LOG(DEBUG,"Sending INTERESTED message to " + printPeerInfo());
  c->writeConn(buff, buff_size);
}


void Peer::sendHave(int piece){

  int length = 5;
  uint8_t msgType = BT_HAVE; 
  ConnectionHandler* c = (ConnectionHandler*)connection;
  int buff_size = length+sizeof(int);
  char buff[buff_size];
  char *runner = (char*)buff;
  
  memcpy((void*)runner, (const void*)&length, sizeof(int));
  runner = runner + sizeof(int);

  memcpy((void*)runner,(const void*)&msgType, sizeof(uint8_t));
  runner = runner + sizeof(uint8_t);

  memcpy((void*)runner,(const void*)&piece, sizeof(int));

  LOG(DEBUG,"Sending HAVE message to " + printPeerInfo());
  c->writeConn(buff, buff_size);
}

void Peer::sendRequest(int index, int begin, int len){

  int length = 13;
  uint8_t msgType = BT_REQUEST; 
  ConnectionHandler* c = (ConnectionHandler*)connection;
  int buff_size = length+sizeof(int);
  char buff[buff_size];
  char *runner = (char*)buff;
  
  memcpy((void*)runner, (const void*)&length, sizeof(int));
  runner = runner + sizeof(int);

  memcpy((void*)runner,(const void*)&msgType, sizeof(uint8_t));
  runner = runner + sizeof(uint8_t);

  memcpy((void*)runner,(const void*)&index, sizeof(int));
  runner = runner + sizeof(int);

  memcpy((void*)runner,(const void*)&begin, sizeof(int));
  runner = runner + sizeof(int);

  memcpy((void*)runner,(const void*)&len, sizeof(int));
  
  LOG(DEBUG,"REQUESTING Piece#" + to_string(index) + " Block#" + to_string(begin/BLOCK_SIZE) + " from " + printPeerInfo());

	lastCommunicationTime = clock();//chrono::high_resolution_clock::now();

  c->writeConn(buff, buff_size);
}

void Peer::sendPiece(int index, int begin, const char *block, size_t size){

  uint8_t msgType = BT_PIECE;
  int length = 9 + size;
  ConnectionHandler* c = (ConnectionHandler*)connection;
  int buff_size = length+sizeof(int);
  char buff[buff_size];
  char *runner = (char*)buff;
  
  memcpy((void*)runner, (const void*)&length, sizeof(int));
  runner = runner + sizeof(int);

  memcpy((void*)runner,(const void*)&msgType, sizeof(uint8_t));
  runner = runner + sizeof(uint8_t);

  memcpy((void*)runner,(const void*)&index, sizeof(int));
  runner = runner + sizeof(int);

  memcpy((void*)runner,(const void*)&begin, sizeof(int));
  runner = runner + sizeof(int);

  memcpy((void*)runner,(const void*)block, size);

  LOG(DEBUG,"Sending PIECE Piece#" + to_string(index) + " Block#" + to_string(begin/BLOCK_SIZE) + " to " + printPeerInfo());
  c->writeConn(buff, buff_size);

	given += (size_t) size;
	//auto elapsed = chrono::high_resolution_clock::now() - lastCommunicationTime;
	totalCommunicationTime += clock() - lastCommunicationTime;//chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
	lastCommunicationTime = clock();//chrono::high_resolution_clock::now();
}

string Peer::status(size_t &totalDownloaded, size_t &totalUploaded)
{
	totalDownloaded = taken;
	totalUploaded = given;
	float totalTime = (float) totalCommunicationTime / CLOCKS_PER_SEC;
	if (totalTime - 0 < 0.001)
		totalTime = 0.001;

	string sts;
	if (totalDownloaded != 0)
	{
		sts += "Downloaded " + to_string(totalDownloaded) + " bytes at " + to_string(((double) totalDownloaded / (1024 * 1024)) / totalTime) + " MB/s from " + printPeerInfo() + ". ";
	}
	if (totalUploaded != 0)
	{
		sts += "Uploaded " + to_string(totalUploaded) + " bytes at " + to_string(((double) totalUploaded / (1024 * 1024)) / totalTime) + " MB/s from " + printPeerInfo() + ". ";
	}
	return sts;
}
