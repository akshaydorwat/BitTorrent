#ifndef REQUEST_PROCESSOR_HPP
#define REQUEST_PROCESSOR_HPP

#include "ThreadPool.h"
#include "Piece.hpp"
#include "Peer.hpp"

#define POOL_SIZE 5

class RequestProcessor{

public:

  RequestProcessor( vector<Piece *> *p){
    pieces = p;
    requestServer =  new ThreadPool(POOL_SIZE);
  }

  ~RequestProcessor(){
    delete requestServer;
  }

  void addTask(int index, int begin, int len, Peer *p);
   
private:
  void handleRequest(int index, int begin, int len, Peer *p);

  ThreadPool *requestServer;
  vector<Piece *> *pieces;
};

#endif
