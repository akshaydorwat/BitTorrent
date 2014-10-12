/*
 *  PieceProcessor.cpp
 *
 *  Created on: Oct 11, 2014
 *      Author: rkhapare
 */

#include "PieceProcessor.hpp"
#include "PieceRequestor.hpp"
#include "Piece.hpp"
#include "Peer.hpp"
#include "Logger.hpp"
#include "ThreadPool.h"

#include <string>
#include <vector>
using namespace std;

PieceProcessor::PieceProcessor(vector<Piece*> &pieces, PieceRequestor *pieceRequestor)
:pieces(pieces)
,pieceRequestor(pieceRequestor)
{	
	pool = new ThreadPool(PIECEPROCESSOR_POOL_SIZE);
}

PieceProcessor::~PieceProcessor()
{
	delete pool;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PieceProcessor::addTask(int index, int begin, string block, void *peer)
{
	pool->enqueue(std::bind( &PieceProcessor::handlePiece, this, (size_t)index, (size_t)begin, block, peer));	
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PieceProcessor::handlePiece(size_t pieceId, size_t blockOffset, string blockData, void *peer)
{
	pieceRequestor->signalGoAhead((Peer *)peer); // notify the requestor before processing the incoming piece

	if (pieceId < pieces.size() && blockOffset/BLOCK_SIZE < pieces[pieceId]->numOfBlocks())
	{
		pieces[pieceId]->setBlockByOffset(blockOffset, blockData.size(), blockData);
		pieces[pieceId]->writeContiguousBlocksToDisk();		// lazy write piece data to disk
	}
	else
	{
		LOG(ERROR, "PieceProcessor : Piece index or Block offset out of range. Piece#" + to_string(pieceId) + " of total pieces " + to_string(pieces.size()) + " Block#" + to_string(blockOffset/BLOCK_SIZE) + " offset (" + to_string(blockOffset) + ")");
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
