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
#include <time.h>
using namespace std;

///////////////////////////////////////////////////////////////////////////////////////////////////
PieceRequestor::PieceRequestor(vector<Piece*> &pieces, vector<void*> &peers)
	:pieces(pieces)
	 ,peers(peers)
{
	for (size_t i=0; i < pieces.size(); i++)
		pieceProcessing.push_back(false);
	firstRequestSent = false;
	runTime = 0;

	updateSts = true;	// NON-BLOCKING TEST-AND-RESET lock so only one thread prints status at a time
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PieceRequestor::startPieceRequestor()
{
	size_t requestPieceId;
	size_t requestBlockBegin;
	size_t requestBlockLength;
	unsigned char *peerId;

	LOG (DEBUG, "PieceRequestor : Starting ...");
	terminated = false;

	while(!allPiecesAvailable())
	{
		while (!unavailablePieceIsServicable() && !terminated)
		{
			LOG(DEBUG, "PieceRequestor : No unavailable piece is currently servicable by the Peer list. Sleeping ...");
			this_thread::sleep_for(chrono::seconds(3));
		}

		waitForGoAhead();
		if (terminated || allPiecesAvailable())
			break;

		if(selectRandomUnavailableUnprocessedPiece(requestPieceId, requestBlockBegin, requestBlockLength, &peerId))
		{
			pieces[requestPieceId]->setBlockProcessing(requestBlockBegin/BLOCK_SIZE);

			for (size_t p=0; p<peers.size() && !terminated; p++)
			{
				Peer *peer = (Peer *)peers[p];

				if (memcmp((void*)peer->getId(),(void *)peerId, ID_SIZE) == 0)
				{
					setPieceProcessing(requestPieceId);

					requestedPeerIds.push_back(peerId);
					requestTimestamps.push_back(chrono::duration_cast<std::chrono::milliseconds>(chrono::steady_clock::now().time_since_epoch()).count());
					requestedPieceIds.push_back(requestPieceId);
					requestedBlockIds.push_back(requestBlockBegin / BLOCK_SIZE);
					
					if (!terminated)
					{
						if (!firstRequestSent)
						{
							firstRequestSent = true;
							firstRequestBegin = chrono::duration_cast<std::chrono::milliseconds>(chrono::steady_clock::now().time_since_epoch()).count();	
							//clock();	// Start stopwatch at start of first request
							runTime = firstRequestBegin;
						}
						//cout << "PieceRequestor : Sending REQUEST to PeerId#";
						//print_peer_id((unsigned char *)peer->getId());
						//cout << endl;
						//cout << "RequestedPeerIds.size() = " << requestedPeerIds.size() << endl;
						//LOG (DEBUG, "PieceRequestor : Sending REQUEST for Piece#" + to_string(requestPieceId) + " Block#" + to_string(requestBlockBegin/BLOCK_SIZE));
						
						peer->sendRequest(requestPieceId, requestBlockBegin, requestBlockLength);
					}
					break;
				}
			}
		}
		else
		{
			LOG (DEBUG, "PieceRequestor : Unable to formulate new REQUEST. Sleeping ... ");
			this_thread::sleep_for(chrono::seconds(3));
		}

		if (terminated)
			break;
	}

	if (!terminated)
		LOG (DEBUG, "PieceRequestor : All pieces available/received and verified. Terminating PieceRequestor !!!");
	else
		LOG (INFO, "PieceRequestor : Received terminate signal. Terminating PieceRequestor !!!");
}

///////////////////////////////////////////////////////////////////////////////////////////////////
string PieceRequestor::status(size_t &totalBlocksCompleted, size_t &totalBlocks)
{
	if (!updateSts)
		return "";
	updateSts = false;
	
	totalBlocksCompleted = 0;
	totalBlocks = 0;
	for (size_t i=0; i<pieces.size(); i++)
	{
		size_t blocksCompleted;
		string sts = pieces[i]->status(blocksCompleted);
		totalBlocksCompleted += blocksCompleted;
		totalBlocks += pieces[i]->numOfBlocks();

		LOG (DEBUG, sts);
	}
	string sts = to_string(totalBlocksCompleted) + " of " + to_string(totalBlocks) + " blocks available. ~" + to_string((100 * totalBlocksCompleted) / totalBlocks) + "%";

	LOG (INFO, "PieceRequestor : " + sts);

	updateSts = true;
	return sts;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool PieceRequestor::allPiecesAvailable()
{
	runTime = chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now().time_since_epoch()).count() - firstRequestBegin;
	//runTime = clock();	// keep updating the runTime thus far
	//LOG (DEBUG, to_string(runTime) + " clocks " + to_string(CLOCKS_PER_SEC) + " clocks per second.");
	//LOG (DEBUG, "PieceRequestor : Running since " + to_string(getRunTime() / 1000.0) + " seconds.");
	
	for (size_t i=0; i<pieces.size(); i++){
		if (!pieces[i]->isValid())
		{
			//LOG (DEBUG, "PieceRequestor : Continue waiting for Piece#" + to_string(i));
			return false;
		}
	}
	
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool PieceRequestor::unavailablePieceIsServicable()
{
	for (size_t i=0; i<pieces.size(); i++)
	{
		unsigned char *peerId;
		if (!pieces[i]->isValid() && selectServicablePeer(i, &peerId))
		{
			//LOG (DEBUG, "PieceRequestor : Piece#" + to_string(i) + " now servicable.");
			return true;
		}
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PieceRequestor::waitForGoAhead()
{
	signalTimeout();
	while (requestedPeerIds.size() == MAX_REQUESTS && !terminated){
		//LOG (DEBUG, "PieceRequestor : MAX_REQUESTS(" + to_string(MAX_REQUESTS) + ") formulated. Waiting for GO_AHEAD ...");
		this_thread::sleep_for(chrono::seconds(1));
		signalTimeout();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PieceRequestor::signalTimeout()
{
	size_t nowInMillis = chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now().time_since_epoch()).count();
	
	requestMtx.lock();
	size_t i = 0;
	for (; i < requestedPeerIds.size(); i++)
	{
		if (nowInMillis - requestTimestamps[i] >= REQUEST_TIMEOUT_MILLIS)
		{
			//LOG (WARNING, "PieceRequestor : Request #" + to_string(i) + " timed out. Purging !!!");
			requestedPeerIds.erase(requestedPeerIds.begin() + i);
			requestTimestamps.erase(requestTimestamps.begin() + i);
			pieces[requestedPieceIds[i]]->resetBlockProcessing(requestedBlockIds[i]);
			requestedPieceIds.erase(requestedPieceIds.begin() + i);
			requestedBlockIds.erase(requestedBlockIds.begin() + i);
		}
		else break;	// requests are queued; so following requests must have obviously been added at a later time
	}
	
	if (i > 0) LOG (WARNING, "PieceRequestor : Purged " + to_string(i) + " timed-out REQUEST(S).");
	requestMtx.unlock();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PieceRequestor::signalGoAhead(void *peerPtr)
{
	Peer *peer = (Peer *)peerPtr;
	//cout << "PieceRequestor : signalling reception from PeerId#";
	//print_peer_id((unsigned char *)peer->getId());
	//cout << endl;

	//LOG (DEBUG, "PieceProcessor : LOCK.");
	requestMtx.lock();
	for (size_t i=0; i < requestedPeerIds.size(); i++)
	{
		if(memcmp((void *)requestedPeerIds[i], (void *)peer->getId(), ID_SIZE) == 0)
		{
			requestedPeerIds.erase(requestedPeerIds.begin() + i);
			requestTimestamps.erase(requestTimestamps.begin() + i);
			requestedPieceIds.erase(requestedPieceIds.begin() + i);
			requestedBlockIds.erase(requestedBlockIds.begin() + i);
			break;
		}
	}
	requestMtx.unlock();
	//LOG (DEBUG, "PieceProcessor : UNLOCK.");
	//size_t totalBlocksCompleted, totalBlocks;
	//status(totalBlocksCompleted, totalBlocks);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
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
	{
		//LOG (WARNING, "PieceRequestor : No servicablePeer found for Piece#" + to_string(pieceId));
		return false;
	}
	else if (servicablePeerIds.size() == 1)
	{
		*peerId = servicablePeerIds[0];
		//LOG (DEBUG, "PieceRequestor : The only servicablePeer re-chosen for Piece#" + to_string(pieceId));
	}
	else if (servicablePeerIds.size() > 1)
	{
		//LOG (DEBUG, "PieceRequestor : LOCK.");
		requestMtx.lock();
		size_t randomId = random() % servicablePeerIds.size();
		for (size_t p=0; p<servicablePeerIds.size(); p++)
		{
			size_t peerIdx = (randomId + p) % servicablePeerIds.size();						
			if (p+1 == servicablePeerIds.size()) // if this is the only servicable peer for this piece
			{
				*peerId = servicablePeerIds[peerIdx];	// choose/make do with him even if he is overloaded
				//LOG (DEBUG, "PieceRequestor : Last servicablePeer chosen for Piece#" + to_string(pieceId));
				break;
			}

			size_t r=0;
			for (r=0; r<requestedPeerIds.size(); r++)
			{
				if (memcmp((void *)servicablePeerIds[peerIdx], (void *)requestedPeerIds[r], ID_SIZE) == 0)
					break; // this peer has already been requested
			}
			if (r==requestedPeerIds.size())	// This peer has not already been requested
			{
				*peerId = servicablePeerIds[peerIdx];
				//LOG (DEBUG, "PieceRequestor : New servicablePeer chosen for Piece#" +to_string(pieceId));
				break;
			}
		}
		requestMtx.unlock();
		//LOG (DEBUG, "PieceRequestor : UNLOCK.");
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/*
 * Random piece selector : must be followed by the setPieceProcessing method
 * NOTE: the blockOffset specifies the 0 based offset of a block within the selected piece
 */
bool PieceRequestor::selectRandomUnavailableUnprocessedPiece(size_t &pieceId, size_t& blockOffset, size_t& blockLength, unsigned char **peerId)
{
	if (allPiecesAvailable())
		return false;

	bool found = false;
	size_t numOfPieces = pieces.size();

	size_t randomId = random() % numOfPieces;

	if (requestedPeerIds.size() % 2 == 1 || randomId % 2 == 1)
	{
		// first, check for a new block to be requested from an already processing piece
		for (size_t i = 0; i < pieces.size(); i++)
		{
			pieceId = i;
			if (pieceProcessing[pieceId])	// If piece not already available but is already processing
			{
				//LOG(DEBUG, "PieceRequestor : Chose already processing Piece#" + to_string(pieceId));
				found = pieces[pieceId]->selectUnavailableUnprocessedBlock(blockOffset, blockLength);
				if (found)
					found = found && selectServicablePeer(pieceId, peerId); // even a different peer may be chosen for requesting a new block
				if (found)
				{	
					//LOG(DEBUG, "PieceRequestor : Unavailable, Processing Piece#" + to_string(pieceId) +" : block#" + to_string(blockOffset/BLOCK_SIZE) + " [" + to_string(blockOffset) + " - " + to_string(blockLength) + "]");
					break;
				}
			}
		}
	}
	// second, check for a new piece to be requested 
	if (!found)
	{
		//LOG (DEBUG, "PieceRequestor : Unavailable, Processing Piece not found. Looking for Unprocessed Piece.");
		//LOG (DEBUG, "PieceRequestor : Choosing Piece using on randomId#" + to_string(randomId));
		for (size_t i = 0; i < numOfPieces; i++)
		{
			pieceId = (randomId + i) % numOfPieces;
			if (!isPieceAvailable(pieceId))	// If piece not already available and not already processing
			{
				//LOG (DEBUG, "PieceRequestor : Unavailable, Unprocessed Piece#" + to_string(pieceId));
				found = pieces[pieceId]->selectUnavailableUnprocessedBlock(blockOffset, blockLength);
				if (found)
					found = found && selectServicablePeer(pieceId, peerId);
				if (found)
				{
					//LOG(DEBUG, "PieceRequestor : Unavailable, Unprocessed Piece#" + to_string(pieceId) +" : block#" + to_string(blockOffset/BLOCK_SIZE) + " [" + to_string(blockOffset) + " - " + to_string(blockLength) + "]");
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
		//LOG (DEBUG, "PieceRequestor : Piece#" + to_string(pieceId) +" status-update = processing.");
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PieceRequestor::resetPieceProcessing(size_t pieceId)
{
	if (pieces.size() > pieceId)
	{
		pieceProcessing[pieceId] = false;
		//LOG (DEBUG, "PieceRequestor : Piece#" + to_string(pieceId) +" status-update = not processing.");
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool PieceRequestor::isPieceProcessing(size_t pieceId)
{
	if (pieces.size() > pieceId)
	{
		//if (pieceProcessing[pieceId])
		//LOG (DEBUG, "PieceRequestor : Piece#" + to_string(pieceId) +" status-check = processing.");
		//else
		//LOG (DEBUG, "PieceRequestor : Piece#" + to_string(pieceId) +" status-check = not processing.");
		return pieceProcessing[pieceId];
	}
	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
