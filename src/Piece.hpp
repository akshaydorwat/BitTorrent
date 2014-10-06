/*
 * Piece.hpp
 *
 *  Created on: Oct 04, 2014
 *      Author: rkhapare
 */

#ifndef PIECE_HPP_
#define PIECE_HPP_

#include <string>
#include <vector>
using namespace std;

#define BLOCK_SIZE 256000 //characters

class Piece
{
 private:
  string data;

  size_t id;
  size_t offset;
  size_t length;
  vector<int> blockAvailable;
  vector<int> blockProcessing;
  //vector<TorrentPieceBlock_t> blocks;
  //string hash;

 public:
  Piece(size_t id, size_t offset, size_t length);// : id(id), offset(offset), length(length) {} // Piece Id, Start offset, Length
  ~Piece();

  size_t getId() { return id; }
  size_t getStartOffset() { return offset; }
  size_t getLength() { return length; }
  //void setLength(size_t);

  void setData(string);
  string getData() { return data; }
  bool isComplete() { return length == data.size(); }
  size_t numOfBlocks() { return 1 + length / BLOCK_SIZE; }
  bool selectUnavailableUnprocessedBlock(size_t&, size_t&);
  void setBlockProcessing(size_t);
  void resetBlockProcessing(size_t);
  bool isBlockProcessing(size_t);
  void setBlockById(size_t, string);
  void setBlockByOffset(size_t, size_t, string);
  bool isBlockAvailable(size_t);
  void setBlockAvailable(size_t);
  void resetBlockAvailable(size_t);
  
  //void setHash(string);
  //string getHash() { return hash; }
  bool isValid();
};

#endif /* PIECE_HPP_ */
