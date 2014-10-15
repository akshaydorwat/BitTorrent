/*
 * PieceProcessor.hpp
 *
 *  Created on: Oct 11, 2014
 *      Author: rkhapare
 */

#ifndef PIECEPROCESSOR_HPP_
#define PIECEPROCESSOR_HPP_

#include "Piece.hpp"
#include "PieceRequestor.hpp"
#include "ThreadPool.h"

#include <string>
#include <vector>
using namespace std;

///////////////////////////////////////////////////////////////////////////////////////////////////
#define PIECEPROCESSOR_POOL_SIZE 1

///////////////////////////////////////////////////////////////////////////////////////////////////
class PieceProcessor
{
 private:
  vector<Piece*> &pieces; 
  PieceRequestor *pieceRequestor;
  ThreadPool *pool;
  void handlePiece(size_t, size_t, string, void *);

 public:
  PieceProcessor(vector<Piece*> &, PieceRequestor *);
  ~PieceProcessor();
  void addTask(int, int, string, void *);
};

#endif /* PIECEPROCESSOR_HPP_ */
///////////////////////////////////////////////////////////////////////////////////////////////////
