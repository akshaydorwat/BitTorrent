/*
 * PieceRequestor.cpp
 *
 *  Created on: Oct 11, 2014
 *      Author: rkhapare
 */

#include "PieceRequestor.hpp"
#include "Piece.hpp"
#include "Peer.hpp"
#include "Logger.hpp"
#include "bt_lib.h"
#include <string>
#include <vector>
#include <thread>
#include <chrono>

using namespace std;

PieceRequestor::PieceRequestor(vector<Piece*> &pieces, vector<void*> &peers)
  :pieces(pieces)
  ,peers(peers)
{
  for (size_t i=0; i < pieces.size(); i++)
    {
      if (pieces[i]->isValid())
	pieceAvailable.push_back(true);	
      else
	pieceAvailable.push_back(false);
		
      pieceProcessing.push_back(false);
      pieceDirty.push_back(false);
    }
}

void PieceRequestor::startPieceRequestor()
{
  size_t requestPieceId;
  size_t requestBlockBegin;
  size_t requestBlockLength;
  unsigned char *peerId;
  LOG (DEBUG, "PieceRequestor : Starting to formulate new REQUESTS.");
  while(!allPiecesAvailable()){
    if(selectRandomUnavailableUnprocessedPiece(requestPieceId, requestBlockBegin, requestBlockLength, &peerId))
    {
      LOG (DEBUG, "PieceRequestor : while loop.");
      waitForGoAhead();
      pieces[requestPieceId]->setBlockProcessing(requestBlockBegin/BLOCK_SIZE);
      for (size_t p=0; p<peers.size(); p++)
	{
	Peer *peer = (Peer *)peers[p];
	if (memcmp((void*)peer->getId(),(void *)peerId, ID_SIZE) == 0)
	  {
	    requestedPeerIds.push_back(peerId);
	    LOG(DEBUG,"Ready to send REQUEST message");
	    peer->sendRequest(requestPieceId, requestBlockBegin, requestBlockLength);
	    break;
	  }
	}
    }
    else{
      LOG (DEBUG, "PieceRequestor : Unable to formulate new REQUEST. Sleeping ... ");
      this_thread::sleep_for(chrono::seconds(1));
    }
  }
  LOG (DEBUG, "PieceRequestor : All pieces available/received. Terminating.");
}

bool PieceRequestor::allPiecesAvailable()
{
  for (size_t i=0; i<pieces.size(); i++){
    if (!pieces[i]->isValid())
      return false;
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PieceRequestor::waitForGoAhead()
{
  while (requestedPeerIds.size() == MAX_REQUESTS){
    LOG (DEBUG, "PieceRequestor : MAX_REQUESTS formulated. Waiting for GO_AHEAD ...");
    this_thread::sleep_for(chrono::seconds(1));
  }
}

void PieceRequestor::signalGoAhead(void *peerPtr)
{
  Peer *peer = (Peer *)peerPtr;
  requestMtx.lock();
  for (size_t i=0; i < requestedPeerIds.size(); i++)
    {
      if(memcmp((void *)requestedPeerIds[i], (void *)peer->getId(), ID_SIZE) == 0)	// comparing unsigned char * .. duh
	{
	  requestedPeerIds.erase(requestedPeerIds.begin() + i);
	  /*
	    vector<unsigned char*>::iterator it = requestedPeerIds.begin();
	    advance(it, pos);
	    requestedPeerIds.erase(it);
	  */
	  break;
	}
    }
  requestMtx.unlock();
}

bool PieceRequestor::selectServicablePeer(size_t pieceId, unsigned char **peerId)
{
  // check whether some unchoked peer/seeder possesses this piece at all
  vector<unsigned char *> servicablePeerIds;
  for (size_t p=0; p < peers.size(); p++)
    {
      Peer *peer = (Peer *) peers[p];
      if (peer->isConnectionEstablished() && !peer->isChocked() && peer->getBitVector(pieceId)){ // && peer->containsPiece(pieceId)
	servicablePeerIds.push_back( peer->getId());
      }
    }
  if (servicablePeerIds.size() == 0)	// no unchoked, connected peer can currently service this piece
    return false;
  else if (servicablePeerIds.size() == 1){
    *peerId = servicablePeerIds[0];
  }
  else if (servicablePeerIds.size() > 1)
    {
      requestMtx.lock();
      for (size_t p=0; p<servicablePeerIds.size(); p++)
	{						
	  if (p+1 == servicablePeerIds.size()) // if this is the only servicable peer for this piece
	    {
	      //peerId = servicablePeerIds[p];	// choose/make do with him even if he is overloaded
	      break;
	    }

	  for (size_t r=0; r<requestedPeerIds.size(); r++)
	    {
	      if (memcmp((void *)servicablePeerIds[p], (void *)requestedPeerIds[r], ID_SIZE) == 0)
		break; // this peer has already been requested
	    }
	}	
      requestMtx.unlock();
    }
  return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Random piece selector : must be followed by the setPieceProcessing method
// NOTE: the blockOffset specifies the 0 based offset of a block within the selected piece
bool PieceRequestor::selectRandomUnavailableUnprocessedPiece(size_t &pieceId, size_t& blockOffset, size_t& blockLength, unsigned char **peerId)
{
  bool found = false;
  size_t numOfPieces = pieces.size();
  for (size_t i=0; i < numOfPieces; i++)
    {
      if (pieces[i]->isValid())
	{
	  if (i+1 == numOfPieces)
	    return false;
	}
    }

  // first, check for a new block to be requested from an already processing piece
  for (size_t i = 0; i < pieces.size(); i++)
    {
      if (!pieceAvailable[i] && pieceProcessing[i])	// If piece not already available but is already processing
	{
	  pieceId = i;
	  found = pieces[i]->selectUnavailableUnprocessedBlock(blockOffset, blockLength);
	  found = found && selectServicablePeer(pieceId, peerId); // even a different peer may be chosen for requesting a new block
	  if (found)
	    {	
	      LOG(DEBUG, "PieceRequestor : Unavailable, Processing Piece#" + to_string(pieceId) +" : block#" + to_string(blockOffset/BLOCK_SIZE) + " [" + to_string(blockOffset) + " - " + to_string(blockLength) + "]");
	      break;
	    }
	}
    }

  // second, check for a new piece to be requested 
  if (!found)
    {
      size_t randomId = random() % numOfPieces;

      for (size_t i = (randomId + 1) % numOfPieces; i != randomId; i = (i + 1) % numOfPieces)
	{
	  if (!pieceAvailable[i] && !pieceProcessing[i])	// If piece not already available and not already processing
	    {
	      pieceId = i;
	      found = pieces[i]->selectUnavailableUnprocessedBlock(blockOffset, blockLength);
	      found = found && selectServicablePeer(pieceId, peerId);
	      if (found)
		{
		  LOG(DEBUG, "PieceRequestor : Unavailable, Unprocessed Piece#" + to_string(pieceId) +" : block#" + to_string(blockOffset/BLOCK_SIZE) + " [" + to_string(blockOffset) + " - " + to_string(blockLength) + "]");
		  break;
		}
	    }
	}
    }

  return found;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PieceRequestor::setPieceProcessing(size_t pieceId)
{
  if (!isPieceAvailable(pieceId))
    {
      pieceProcessing[pieceId] = true;	
      LOG (DEBUG, "PieceRequestor : Piece#" + to_string(pieceId) +" status-update = processing.");
    }
}

void PieceRequestor::resetPieceProcessing(size_t pieceId)
{
  if (pieces.size() > pieceId)
    {
      pieceProcessing[pieceId] = false;
      LOG (DEBUG, "PieceRequestor : Piece#" + to_string(pieceId) +" status-update = not processing.");
    }
}

bool PieceRequestor::isPieceProcessing(size_t pieceId)
{
  if (pieces.size() > pieceId)
    {
      if (pieceProcessing[pieceId])
	LOG (DEBUG, "PieceRequestor : Piece#" + to_string(pieceId) +" status-check = processing.");
      else
	LOG (DEBUG, "PieceRequestor : Piece#" + to_string(pieceId) +" status-check = not processing.");
      return pieceProcessing[pieceId];
    }
  return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
