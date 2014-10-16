/*
 * Piece.cpp
 *
 *  Created on: Oct 04, 2014
 *      Author: rkhapare
 */

#include "Piece.hpp"
#include "FileHandler.hpp"
#include "Logger.hpp"

#include <string>
#include <vector>
#include <mutex>
#include <openssl/sha.h>
using namespace std;

///////////////////////////////////////////////////////////////////////////////////////////////////
mutex availableMtx, processingMtx, dirtyMtx;

	Piece::Piece(size_t id, size_t offset, size_t length, string hash, FileHandler *fileMgr)
	:id(id)
	,offset(offset)
	,length(length)
	,hash(hash)
	 ,fileMgr(fileMgr)
{
	//data = new char[length];
	for (size_t i=0; i < numOfBlocks(); i+=sizeof(int))
	{
		blockAvailable.push_back(0);
		blockProcessing.push_back(0);
		blockDirty.push_back(0);
	}
	valid = false;
	//LOG (DEBUG, "Piece#" + to_string(id) + " numOfBlocks=" + to_string(numOfBlocks()) + " length=" + to_string(length));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
Piece::~Piece()
{
	//	delete[] data;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
size_t Piece::numOfBlocks()
{
	size_t numBlocks = length / BLOCK_SIZE;
	if (length % BLOCK_SIZE > 0)
		numBlocks++;
	return numBlocks;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
string Piece::status(size_t &count)
{
	count = 0;
	string sts ("Piece#" + to_string(id) + " : [");
	for (size_t i=0; i<numOfBlocks(); i++)
	{
		if(isBlockAvailable(i))
		{
			count++;
			sts += "#";
		}
		else
			sts += "-";
	}
	sts += "] " + to_string(count) + "/" + to_string(numOfBlocks()) + " ~" + to_string((100 * count) / numOfBlocks()) + "%";
	return sts;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Piece::setData(string str)
{
	if (str.size() > getLength())
		str = str.substr(0, getLength());

	size_t strSize = str.size();
	for (size_t i=0; i < strSize; i+=BLOCK_SIZE)
	{
		size_t blockSize = strSize - i > BLOCK_SIZE ? BLOCK_SIZE : strSize - i;
		/* NOTE: setData() is to be used for prepopulating data. 
		   So the dirty flags have been cleared immediately following updating availability. */
		setBlockProcessing(i/BLOCK_SIZE);
		setBlockByOffset(i, blockSize, str.substr(i, blockSize));
		resetBlockProcessing(i/BLOCK_SIZE);
		resetBlockDirty(i/BLOCK_SIZE);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/*
 * Random block selector : must be followed by the setBlockProcessing method
 */
bool Piece::selectUnavailableUnprocessedBlock(size_t& blockOffset, size_t& blockLength)
{
	//LOG (DEBUG, "Piece#" + to_string(id) + " : Choosing from numOfBlocks=" + to_string(numOfBlocks()) + " max mod " + to_string(16%numOfBlocks()));
	bool found = false;
	if (isValid() || numOfBlocks() == 0)
		return found;

	size_t randomId = random() % numOfBlocks();
	size_t blockId = 0;

	availableMtx.lock();
	processingMtx.lock();

	int availableSubbitmap = 0;
	int processingSubbitmap = 0;
	int lvl1Offset = 0;
	int lvl2Offset = 0;

	for (size_t i = 0; i < numOfBlocks(); i++)
	{
		blockId = ((randomId + i) % numOfBlocks());

		lvl1Offset = blockId / sizeof(int);
		lvl2Offset = blockId % sizeof(int);

		availableSubbitmap = blockAvailable[ lvl1Offset ];
		int temp = availableSubbitmap & (1 << lvl2Offset);
		if (temp == 0)	// If block not already available
		{
			processingSubbitmap = blockProcessing[ lvl1Offset ];
			temp = processingSubbitmap & (1 << lvl2Offset);
			if (temp == 0)	// If block not already in processing queue
			{
				blockOffset = blockId * BLOCK_SIZE;
				blockLength = blockId+1 < numOfBlocks() ? BLOCK_SIZE : length - blockOffset;
				//LOG (DEBUG, "Piece#" + to_string(id) + " Length " + to_string(length) + " blockOffset " + to_string(blockOffset) + " blockLength " + to_string(blockLength));
				found = true;
				//LOG(DEBUG, "Piece#" + to_string(id) +" : Unavailable, Unprocessed block#" + to_string(blockId) + " [" + to_string(blockOffset) + " - " + to_string(blockLength) + "]");
				break;
			}
		}
	}

	availableMtx.unlock();
	processingMtx.unlock();

	return found;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/* 
 * Lazy file-writer :  	Attempts to write chunks of prespecified number of blocks to disk
 *		 	The lazy-write policy is dropped if piece isValid
 */
void Piece::writeContiguousBlocksToDisk()
{
	if (isDirty())
	{
		vector<size_t> writeBatches;
		if (isValid())
		{
			// write out entire piece as efficiently as possible (some previously available blocks might have to be written)
			for (size_t i=0; i < numOfBlocks(); i++)
			{
				if (isBlockDirty(i))
				{
					// push start of this batch for writing
					writeBatches.push_back((i / CONTIGUOUS_BLOCKS_BATCH_WRITE) * CONTIGUOUS_BLOCKS_BATCH_WRITE);
					// move to end of this batch so i++ goes over to the start of next batch
					i = (((i / CONTIGUOUS_BLOCKS_BATCH_WRITE) + 1) * CONTIGUOUS_BLOCKS_BATCH_WRITE) - 1;
				}
			}
		}
		else
		{
			// populate batches of dirty blocks (NOTE: last batch may be smaller than CONTIGUOUS_BLOCKS_BATCH_WRITE)
			vector<size_t> dirtyBlocksStart;

			for (size_t i=0; i < numOfBlocks(); i+=CONTIGUOUS_BLOCKS_BATCH_WRITE)
			{
				if (isBlockDirty(i))
					dirtyBlocksStart.push_back(i);
			}

			if (dirtyBlocksStart.size() > 0)
			{
				for (size_t i=0; i < dirtyBlocksStart.size(); i++)
				{
					bool batchFound = true;
					size_t j = dirtyBlocksStart[i] +1;
					for (size_t k=1; batchFound && k < CONTIGUOUS_BLOCKS_BATCH_WRITE && j < numOfBlocks(); j++, k++)
					{
						if (!isBlockDirty(j))
							batchFound = false;
					}
					if (batchFound)
						writeBatches.push_back(dirtyBlocksStart[i]);
				}
			}
		}

		// if one or more batches are ready to be written, write those blocks to disk
		if (writeBatches.size() > 0)
		{
			for (size_t i=0; i < writeBatches.size(); i++)
			{
				// from start offset of first block in batch
				size_t startBlockOffset = writeBatches[i] * BLOCK_SIZE;
				// to end offset of last block in batch
				size_t endBlockOffset = writeBatches[i] + CONTIGUOUS_BLOCKS_BATCH_WRITE < numOfBlocks() ? (writeBatches[i] + CONTIGUOUS_BLOCKS_BATCH_WRITE) * BLOCK_SIZE : length;
				// perform the batch write to disk
				size_t lengthWritten = fileMgr->writePiece(id, data.substr(startBlockOffset, endBlockOffset - startBlockOffset), endBlockOffset - startBlockOffset, startBlockOffset);
				// if writing was successful
				if (lengthWritten == endBlockOffset - startBlockOffset)
				{
					for (size_t j=0; j < CONTIGUOUS_BLOCKS_BATCH_WRITE; j++)
					{
						if (writeBatches[i]+j >= numOfBlocks()) 
							break;
						// set the written blocks as not-dirty
						resetBlockDirty(writeBatches[i] + j);
					}
				}

			}

			if (isValid() && !isDirty())
				data = "";		// remove data from memory to free up some space

		}
	}
	//LOG (DEBUG, "Piece#" + to_string(id) + " returning from writeToDisk");
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Piece::setBlockProcessing(size_t blockId)
{
	if (blockId < numOfBlocks() && isBlockAvailable(blockId) == false)
	{
		size_t processingLen = (blockId + 1) * BLOCK_SIZE < length ? BLOCK_SIZE : length - (blockId * BLOCK_SIZE);

		string blankBlock (BLOCK_SIZE, ' ');
		while (data.size() < (blockId * BLOCK_SIZE) + processingLen)
		{
			if (data.size() + blankBlock.size() < length)
				data += blankBlock;
			else
				data += blankBlock.substr(0, processingLen);
		}

		int lvl1Offset = blockId / sizeof(int);
		int lvl2Offset = blockId % sizeof(int);

		processingMtx.lock();
		int temp = blockProcessing [ lvl1Offset ];
		temp |= 1 << lvl2Offset;
		blockProcessing [ lvl1Offset ] = temp;		
		processingMtx.unlock();
		LOG (DEBUG, "Piece#" + to_string(id) +" : Block#" + to_string(blockId) + " status-update = processing.");
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Piece::resetBlockProcessing(size_t blockId)
{
	if (blockId < numOfBlocks())
	{
		int lvl1Offset = blockId / sizeof(int);
		int lvl2Offset = blockId % sizeof(int);

		processingMtx.lock();
		int temp = blockProcessing [ lvl1Offset ];
		temp &= ~(1 << lvl2Offset);
		blockProcessing [ lvl1Offset ] = temp;	
		processingMtx.unlock();
		if (isBlockProcessing(blockId))
			LOG (DEBUG, "Piece#" + to_string(id) +" : Block#" + to_string(blockId) + " status-update = failed to make not processing.");
		else
			LOG (DEBUG, "Piece#" + to_string(id) +" : Block#" + to_string(blockId) + " status-update = not processing.");
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool Piece::isBlockProcessing(size_t blockId)
{
	if (blockId < numOfBlocks())
	{
		int lvl1Offset = blockId / sizeof(int);
		int lvl2Offset = blockId % sizeof(int);

		processingMtx.lock();
		int temp = blockProcessing [ lvl1Offset ];
		processingMtx.unlock();
		temp &= 1 << lvl2Offset;
		if (temp > 0){
			//LOG (DEBUG, "Piece#" + to_string(id) +" : Block#" + to_string(blockId) + " status-check = processing.");
		}else{
			//LOG (DEBUG, "Piece#" + to_string(id) +" : Block#" + to_string(blockId) + " status-check = not processing.");  
		}
		return temp > 0;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Piece::setBlockAvailable(size_t blockId)
{
	if (blockId < numOfBlocks())
	{
		int lvl1Offset = blockId / sizeof(int);
		int lvl2Offset = blockId % sizeof(int);

		availableMtx.lock();
		int temp = blockAvailable [ lvl1Offset ];
		temp |= 1 << lvl2Offset;
		blockAvailable [ lvl1Offset ] = temp;	
		availableMtx.unlock();
		LOG (DEBUG, "Piece#" + to_string(id) +" : Block#" + to_string(blockId) + " status-update = available.");
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Piece::resetBlockAvailable(size_t blockId)
{
	if (blockId < numOfBlocks())
	{
		int lvl1Offset = blockId / sizeof(int);
		int lvl2Offset = blockId % sizeof(int);

		availableMtx.lock();
		int temp = blockAvailable [ lvl1Offset ];
		temp &= ~(1 << lvl2Offset);
		blockAvailable [ lvl1Offset ] = temp;	
		availableMtx.unlock();
		if (isBlockAvailable(blockId))
			LOG (DEBUG, "Piece#" + to_string(id) +" : Block#" + to_string(blockId) + " status-update = failed to make not available.");
		else
			LOG (DEBUG, "Piece#" + to_string(id) +" : Block#" + to_string(blockId) + " status-update = not available.");
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool Piece::isBlockAvailable(size_t blockId)
{
	if (blockId < numOfBlocks())
	{
		int lvl1Offset = blockId / sizeof(int);
		int lvl2Offset = blockId % sizeof(int);

		availableMtx.lock();
		int temp = blockAvailable [ lvl1Offset ];
		availableMtx.unlock();
		temp &= 1 << lvl2Offset;
		//if (temp > 0)
			//LOG (DEBUG, "Piece#" + to_string(id) +" : Block#" + to_string(blockId) + " status-check = available.");
		//else
			//LOG (DEBUG, "Piece#" + to_string(id) +" : Block#" + to_string(blockId) + " status-check = not available.");

		return temp > 0;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool Piece::isAvailable()
{
	for (size_t i=0; i < numOfBlocks(); i++)
	{
		if (!isBlockAvailable(i))
			return false;
	}
	//LOG (DEBUG, "Piece#" + to_string(id) + " : status-check = available.");
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
string Piece::getAvailableBlock(size_t blockId)
{
	if (isBlockAvailable(blockId))
	{
		bool isLastBlock = blockId+1 == numOfBlocks();
		size_t blockSize = !isLastBlock ? BLOCK_SIZE : length - (blockId * BLOCK_SIZE);

		if (data.size() < blockId * BLOCK_SIZE + blockSize) // fetch entire piece from disk and return 
		{
			string temp;
			if (!fileMgr->readIfValidPiece(id, temp))
				return "";
			else
				data = temp;
		}
		return data.substr(blockId * BLOCK_SIZE, blockSize);
	}
	return "";
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Piece::setBlockDirty(size_t blockId)
{
	if (blockId < numOfBlocks())
	{
		int lvl1Offset = blockId / sizeof(int);
		int lvl2Offset = blockId % sizeof(int);

		dirtyMtx.lock();
		int temp = blockDirty [ lvl1Offset ];
		temp |= 1 << lvl2Offset;
		blockDirty [ lvl1Offset ] = temp;	
		dirtyMtx.unlock();
		//LOG (DEBUG, "Piece#" + to_string(id) +" : Block#" + to_string(blockId) + " status-update = dirty.");
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Piece::resetBlockDirty(size_t blockId)
{
	if (blockId < numOfBlocks())
	{
		int lvl1Offset = blockId / sizeof(int);
		int lvl2Offset = blockId % sizeof(int);

		dirtyMtx.lock();
		int temp = blockDirty [ lvl1Offset ];
		temp &= ~(1 << lvl2Offset);
		blockDirty [ lvl1Offset ] = temp;	
		dirtyMtx.unlock();
		//if (isBlockDirty(blockId))
		//LOG (DEBUG, "Piece#" + to_string(id) +" : Block#" + to_string(blockId) + " status-update = failed to make not dirty.");
		//else 
		//LOG (DEBUG,"Piece#" + to_string(id) +" : Block#" + to_string(blockId) + " status-update = not dirty.");
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool Piece::isBlockDirty(size_t blockId)
{
	if (blockId < numOfBlocks())
	{
		int lvl1Offset = blockId / sizeof(int);
		int lvl2Offset = blockId % sizeof(int);

		dirtyMtx.lock();
		int temp = blockDirty [ lvl1Offset ];
		dirtyMtx.unlock();
		temp &= 1 << lvl2Offset;
		//if (temp > 0)
		//LOG (DEBUG, "Piece#" + to_string(id) +" : Block#" + to_string(blockId) + " status-check = dirty.");
		//else
		//LOG (DEBUG, "Piece#" + to_string(id) +" : Block#" + to_string(blockId) + " status-check = not dirty.");
		
		return temp > 0;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool Piece::isDirty()
{
	for (size_t i=0; i < numOfBlocks(); i++)
	{
		if (isBlockDirty(i))
		{
			//LOG (DEBUG, "Piece#" + to_string(id) + " : status-check = dirty.");
			return true;
		}
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Piece::setAvailable()
{	
	for (size_t i=0; i < numOfBlocks(); i++)
		setBlockAvailable(i);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Piece::resetAvailable()
{
	for (size_t i=0; i < numOfBlocks(); i++)
		resetBlockAvailable(i);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Piece::setBlockById(size_t blockId, string blockData)
{
	size_t blockOffset = blockId * BLOCK_SIZE;
	//LOG (DEBUG, "Piece#" + to_string(id) + " Block#" + to_string(blockId) + " start-offset : " + to_string(blockOffset));
	if (blockOffset < length && isBlockAvailable(blockId) == 0)
	{
		bool isComplete = true;
		size_t blockLen = blockOffset + BLOCK_SIZE <= length ? BLOCK_SIZE : length - blockOffset;
		//LOG (DEBUG, "Piece#" + to_string(id) + " Block#" + to_string(blockId) + " end-offset : " + to_string(blockOffset + blockLen));
		//LOG (DEBUG, "Piece#" + to_string(id) + " Block#" + to_string(blockId) + " size : " + to_string(blockLen));
		//LOG (DEBUG, "Piece#" + to_string(id) + " Block#" + to_string(blockId) + " size " + to_string(blockLen) + " [" + to_string(blockOffset) + " - " + to_string(blockOffset + blockLen) + "] attempting to write " + to_string(blockData.size()) + " bytes");
		if (blockData.size() >= blockLen)
			blockData = blockData.substr(0, blockLen);
		else
		{
			blockLen = blockData.size();
			isComplete = false;
		}
		//if (isComplete)
		//LOG (DEBUG, "Piece#" + to_string(id) + " : Block#" + to_string(blockId) + " status-update = complete.");
		//else
		//LOG (DEBUG, "Piece#" + to_string(id) + " : Block#" + to_string(blockId) + " status-update = not complete.");
		//string blankBlock (BLOCK_SIZE, '');
		if (data.size() >= blockOffset + blockLen)
		{
			data.replace(blockOffset, blockLen, blockData);
			if (isComplete)
			{
				setBlockAvailable(blockId);
				setBlockDirty(blockId);
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Piece::setBlockByOffset(size_t blockOffset, size_t fillLen, string blockData)
{
	if (blockOffset < length && isBlockAvailable(blockOffset/BLOCK_SIZE) == 0)
	{
		size_t blockId = blockOffset/BLOCK_SIZE;
		size_t blockLen = (blockId + 1) * BLOCK_SIZE <= length ? BLOCK_SIZE : length - (blockId * BLOCK_SIZE);
		fillLen = (blockOffset + fillLen - 1) / BLOCK_SIZE == blockId ? fillLen : blockLen - blockOffset; // any fill length upto the block's size is acceptable
		bool isComplete = (blockOffset + fillLen == (blockId + 1) * BLOCK_SIZE) || (blockOffset + fillLen == length);
		//LOG (DEBUG, "Piece#" + to_string(id) + " : Block#" + to_string(blockId) + " size " + to_string(blockLen) + " [" + to_string(blockOffset) + " - " + to_string(blockOffset + blockLen) + "] attempting to write " + to_string(blockData.size()) + " bytes");
		if (blockData.size() >= fillLen)
			blockData = blockData.substr(0, fillLen);
		else
		{
			fillLen = blockData.size();
			isComplete = false;
		}
		//if (isComplete)
		//LOG (DEBUG, "Piece#" + to_string(id) + " : Block#" + to_string(blockId) + " status-update = complete.");
		//else
		//LOG (DEBUG, "Piece#" + to_string(id) + " : Block#" + to_string(blockId) + " status-update = not complete.");

		if (data.size() >= blockOffset + fillLen)
		{
			data.replace(blockOffset, fillLen, blockData);
			if (isComplete)
			{
				setBlockAvailable(blockId);
				setBlockDirty(blockId);
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/*void Piece::setHash(string pieceHash)
  {
  hash = pieceHash;
  }

  string Piece::getHash()
  {
  return hash;
  }
 */

///////////////////////////////////////////////////////////////////////////////////////////////////
bool Piece::isValid(string hash)
{
	if (!valid && isAvailable() && hash.size() == 20) // isComplete() has been removed from validity to support lazy-fetching of servicable piece
	{
		unsigned char pieceHash[20];
		SHA1 ((unsigned char *)data.c_str(), length, pieceHash);

		valid = fileMgr->checkHashes(pieceHash, (unsigned char *) hash.c_str(), 20);
		//LOG (DEBUG, "Piece#" + to_string(id) + " : hash-check = valid.");
	}
	return valid;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
