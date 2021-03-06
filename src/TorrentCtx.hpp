#ifndef TORRENT_CTX_HPP
#define TORRENT_CTX_HPP

#include <vector>
#include <string>
#include <fstream>
#include <time.h>
#include "bt_lib.h"
#include "Torrent.hpp"
#include "Piece.hpp"
#include "FileHandler.hpp"
#include "PieceRequestor.hpp"
#include "PieceProcessor.hpp"
#include "RequestProcessor.hpp"

using namespace std;

class TorrentCtx{

public:
  
  // Managers
  PieceRequestor *pieceRequestor;       // request message maker  manager
  PieceProcessor *pieceProcessor;       // piece message processing manager 
  RequestProcessor *requestProcessor;   // request message processing manager 
  thread pieceRequestorThread;

  TorrentCtx();
  
  ~TorrentCtx();

  // initialise torrent context
  void init(bt_args_t *args);
  
  // contact tracker and load the peers list
  void contact_tracker(bt_args_t * bt_args);

  // get info hash
  string getInfoHash(){
    return infoHash;
  }

  // get Peer pointer from map 
  void* getPeer(unsigned char *peerId);

  // get peer id 
  unsigned char* getPeerId(){
    return peerId;
  }

  void loadPieceStatus();

  // send bitvector
  char *getPiecesBitVector();
  /*char* getPiecesBitVector(){
    return piecesBitVector;
  }*/

  size_t getBitVectorSize(){
    return bitVectorSize;
  }
  //get number of pieces in the torrent
  size_t getNumOfPieces(){
    return metaData.numOfPieces();
  }

  // init bit set
  void initBitVecor();

  // set bit
  void setbit( size_t b);

  // get bit
  int getbit( size_t b);

  // set complete flag
  void setComplete(bool val){
    complete = val;
  }
  
  // get complete value
  bool isComplete(){
    return complete;
  }
  
private:
  //command line arguments
  unsigned char peerId[20];           	// peer id
  string saveFile;                    	//the filename to save to
  string torrentFile;                 	//torrent file name
  struct sockaddr_in sockaddr;        	//sockaddr for server
  
  // Tracker info
  string infoHash;                    	// info hash
  Torrent metaData;			// static torrent metadata
  vector<Piece *> pieces;		// runtime piece info
  FileHandler *fileMgr;

  // Book keeping
  vector<void*> peers;                  // Peers in torrent
  fstream saveFile_fd;                  // File descriptor
  time_t start_time;                    // start time
  bool complete;                        // donwload complete
  char *piecesBitVector;                // piece bit vector
  size_t bitVectorSize;                 // size of bit vector


};
#endif 
