/*
 * Piece.hpp
 *
 *  Created on: Oct 04, 2014
 *      Author: rkhapare
 */

#ifndef PIECE_HPP_
#define PIECE_HPP_

#include "FileHandler.hpp"

#include <string>
#include <vector>
using namespace std;

#define BLOCK_SIZE 16384 // =16 KB ; 32768 =32 KB
#define CONTIGUOUS_BLOCKS_BATCH_WRITE 1

class Piece
{
 private:
  string data;

  bool valid;
  size_t id;
  size_t offset;
  size_t length;
  string hash;
  FileHandler *fileMgr;
  vector<int> blockAvailable;
  vector<int> blockProcessing;
  vector<int> blockDirty; 

 public:
  Piece(size_t id, size_t offset, size_t length, string, FileHandler *fileMgr);
  ~Piece();

  size_t getId() { return id; }
  size_t getStartOffset() { return offset; }
  size_t getLength() { return length; }
  //void setLength(size_t);

  void setData(string);					// discouraged => to save system memory
  string getData() { return data; }			// discouraged => currently not holding data for already completed blocks

  bool isComplete() { return length == data.size(); } 	// is Piece data complete (no hash check) 
  bool isAvailable();					// are all blocks available (no hash check)
  bool isDirty();					// are there available blocks waiting to be written to disk
  void setAvailable();					// piece available = true
  void resetAvailable();				// piece available = false
  size_t numOfBlocks();// { return (length / BLOCK_SIZE); }

  bool selectUnavailableUnprocessedBlock(size_t&, size_t&);
  void writeContiguousBlocksToDisk();

  void setBlockProcessing(size_t);			// block processing = true
  void resetBlockProcessing(size_t);			// block processing = false
  bool isBlockProcessing(size_t);			// is block processing

  void setBlockById(size_t, string);
  void setBlockByOffset(size_t, size_t, string);

  bool isBlockAvailable(size_t);			// is block available
  void setBlockAvailable(size_t);			// block available = true
  void resetBlockAvailable(size_t);			// block available = false
  string getAvailableBlock(size_t);			// fetch a block if available

  void setBlockDirty(size_t);				// block dirty = true
  void resetBlockDirty(size_t);				// block dirty = false
  bool isBlockDirty(size_t);				// is block dirty

  //void setHash(string);
  string getHash() { return hash; }
  void setValid() { valid = true; }
  bool isValid(string hash);				// isComplete and hash match
  bool isValid() { return isValid(hash); }
};

#endif /* PIECE_HPP_ */
