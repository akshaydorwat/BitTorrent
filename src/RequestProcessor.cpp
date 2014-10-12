#include "RequestProcessor.hpp"
#include "Logger.hpp"

void RequestProcessor::addTask(int index, int begin, int len, Peer *p){
  requestServer->enqueue(std::bind( &RequestProcessor::handleRequest, this, index, begin, len, p));
}

void RequestProcessor::handleRequest(int index, int begin, int len, Peer *peer){

  string pieceData;
  
  if(index > (int) pieces->size()){
    LOG(WARNING, "Invalid request received ");
  }
  
  // get piece object from vector
  Piece *p =  pieces->at((int)index);
  
  // check if piece is available
  if(!p->isAvailable()){
    LOG(WARNING, "Requested block is not available");
    return;
  }
  
  // get actual data from piece

  // send piece message to peer
  peer->sendPiece(index, begin, pieceData.data(), (size_t)pieceData.size());
}
