/*
 * Piece.cpp
 *
 *  Created on: Oct 04, 2014
 *      Author: rkhapare
 */

#include "Piece.hpp"

//#include <iostream>
#include <string>
#include <vector>
#include <mutex>
//#include <ctime>
//#include <exception>
//#include <stdexcept>
//#include <cassert>
//#include <stdlib.h>
using namespace std;

mutex availableMtx, processingMtx;

Piece::Piece(size_t id, size_t offset, size_t length)
:id(id)
,offset(offset)
,length(length)
{
	//data = new char[length];
	for (size_t i=0; i < numOfBlocks(); i+=sizeof(int))
	{
		blockAvailable.push_back(0);
		blockProcessing.push_back(0);
	}
}

/*
Piece::~Piece()
{
	delete[] data;
}*/

//////////////////////////////////////////////////////////////////////////////////////
/*size_t Piece::getLength()
{
	return length;
}

void Piece::setLength(size_t len)
{
	length = len;
}*/

//////////////////////////////////////////////////////////////////////////////////////
/*string Piece::getData()
{
	return data;
}*/

void Piece::setData(string str)
{
	if (str.size() > getLength())
		str = str.substr(0, getLength());
	
	size_t strSize = str.size();
	for (size_t i=0; i < strSize; i+=BLOCK_SIZE)
	{
		size_t blockSize = strSize - i > BLOCK_SIZE ? BLOCK_SIZE : strSize - i;
		setBlockByOffset(i, blockSize, str.substr(i, blockSize));
	}
}

//////////////////////////////////////////////////////////////////////////////////////
/*bool Piece::isComplete()
{
	return length == data.size();
}
*/

// Random block selector : must be followed by the setBlockProcessing method
bool Piece::selectUnavailableUnprocessedBlock(size_t& blockOffset, size_t& blockLength)
{
	bool found = false;
	if (isComplete() || numOfBlocks() == 0)
		return found;

	//size_t numOfBlocks = numOfBlocks();
	size_t randomId = random() % numOfBlocks();

	availableMtx.lock();
	processingMtx.lock();

	int availableSubbitmap = 0;
	int processingSubbitmap = 0;
	int lvl1Offset = 0;
	int lvl2Offset = 0;

	for (size_t i = (randomId + 1) % numOfBlocks(); i != randomId; i = (i + 1) % numOfBlocks())
	{
		lvl1Offset = i / sizeof(int);
		lvl2Offset = i % sizeof(int);

		availableSubbitmap = blockAvailable[ lvl1Offset ];
		int temp = availableSubbitmap & (1 << lvl2Offset);
		if (temp == 0)	// If block not already available
		{
			processingSubbitmap = blockProcessing[ lvl1Offset ];
			temp = processingSubbitmap & (1 << lvl2Offset);
			if (temp == 0)	// If block not already in processing queue
			{
				blockOffset = i * BLOCK_SIZE;
				blockLength = i+1 < numOfBlocks() ? BLOCK_SIZE : length - blockOffset;
				found = true;
				break;
			}
		}
	}

	availableMtx.unlock();
	processingMtx.unlock();

	return found;
}

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
	}
}

void Piece::resetBlockProcessing(size_t blockId)
{
	if (blockId < numOfBlocks())
	{
		int lvl1Offset = blockId / sizeof(int);
		int lvl2Offset = blockId % sizeof(int);

		processingMtx.lock();
		int temp = blockProcessing [ lvl1Offset ];
		temp &= 0 - (1 << lvl2Offset);
		blockProcessing [ lvl1Offset ] = temp;	
		processingMtx.unlock();
	}
}

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
		return temp > 0;
	}
	return false;
}

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
	}
}

void Piece::resetBlockAvailable(size_t blockId)
{
	if (blockId < numOfBlocks())
	{
		int lvl1Offset = blockId / sizeof(int);
		int lvl2Offset = blockId % sizeof(int);

		availableMtx.lock();
		int temp = blockAvailable [ lvl1Offset ];
		temp &= 0 - (1 << lvl2Offset);
		blockAvailable [ lvl1Offset ] = temp;	
		availableMtx.unlock();
	}
}

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
		return temp > 0;
	}
	return false;
}

void Piece::setBlockById(size_t blockId, string blockData)
{
	size_t blockOffset = blockId * BLOCK_SIZE;
	if (blockOffset < length && isBlockAvailable(blockId) == 0)
	{
		bool isComplete = true;
		size_t blockLen = blockOffset + BLOCK_SIZE <= length ? BLOCK_SIZE : length - blockOffset;
		if (blockData.size() > blockLen)
			blockData = blockData.substr(0, blockLen);
		else
		{
			blockLen = blockData.size();
			isComplete = false;
		}

		//string blankBlock (BLOCK_SIZE, '');
		if (data.size() >= blockOffset + blockLen)
		{
			data.replace(blockOffset, blockLen, blockData);
			if (isComplete)
				setBlockAvailable(blockId);
		}
	}
}

void Piece::setBlockByOffset(size_t blockOffset, size_t fillLen, string blockData)
{
	if (blockOffset < length && isBlockAvailable(blockOffset/BLOCK_SIZE) == 0)
	{
		size_t blockId = blockOffset/BLOCK_SIZE;
		size_t blockLen = (blockId + 1) * BLOCK_SIZE <= length ? BLOCK_SIZE : length - (blockId * BLOCK_SIZE);
		fillLen = (blockOffset + fillLen) / BLOCK_SIZE == blockId ? fillLen : blockLen - blockOffset; // any fill length less than the block's size is acceptable
		bool isComplete = (blockOffset + fillLen) * BLOCK_SIZE + 1 == (blockId + 1) * BLOCK_SIZE;

		if (blockData.size() > fillLen)
			blockData = blockData.substr(0, fillLen);
		else
		{
			fillLen = blockData.size();
			isComplete = false;
		}

		if (data.size() >= blockOffset + fillLen)
		{
			data.replace(blockOffset, fillLen, blockData);
			if (isComplete)
				setBlockAvailable(blockId);
		}
	}
}
//////////////////////////////////////////////////////////////////////////////////////
/*void Piece::setHash(string pieceHash)
{
	hash = pieceHash;
}

string Piece::getHash()
{
	return hash;
}
*/
//////////////////////////////////////////////////////////////////////////////////////
bool Piece::isValid()
{
	return isComplete();// && data.size() > hash.size(); // edit : hash comparison
}

///////////////////////////////////////////////////////////////////////////////////////////////////
