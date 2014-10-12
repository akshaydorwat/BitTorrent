#include "RequestProcessor.hpp"
#include "Logger.hpp"
#include "Peer.hpp"

void RequestProcessor::addTask(int index, int begin, int len, void *p){
  requestServer->enqueue(std::bind( &RequestProcessor::handleRequest, this, index, begin, len, p));
}

void RequestProcessor::handleRequest(int index, int begin, int len, void *p){

  string pieceData;
  Peer *peer = (Peer *) p;

  if(index > (int) pieces->size()){
    LOG(WARNING, "Invalid request received ");
  }
  
  // get piece object from vector
  Piece *piece =  pieces->at((int)index);
  
  // check if piece is available
  if(!piece->isAvailable()){
    LOG(WARNING, "Requested block is not available");
    return;
  }
  
  // get actual data from piece

  // send piece message to peer
  peer->sendPiece(index, begin, pieceData.data(), (size_t)pieceData.size());
}
