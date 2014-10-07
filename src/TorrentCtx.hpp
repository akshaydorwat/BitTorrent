#ifndef TORRENT_CTX_HPP
#define TORRENT_CTX_HPP

#include <vector>
#include <string>
#include <time.h>
#include "bt_lib.h"
//#include "Reactor.hpp"
#include "Torrent.hpp"
#include <fstream>

using namespace std;

class TorrentCtx{

public:
  //TorrentCtx(Reactor *r);
  ~TorrentCtx(){
    if(piecesBitVector){
      delete piecesBitVector;
    }
  }

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

  // send bitvector
  char* getPiecesBitVector(){
    return piecesBitVector;
  }

  size_t getBitVectorSize(){
    return bitVectorSize;
  }
  //get number of pieces in the torrent
  size_t getNumOfPieces(){
    return metaData.getNumOfPieces();
  }

  // init bit set
  void initBitVecor();

  // set bit
  void setbit( size_t b);

  // get bit
  int getbit( size_t b);

private:
  //command line arguments
  unsigned char peerId[20];           // peer id
  string saveFile;                    //the filename to save to
  string torrentFile;                 //torrent file name
  struct sockaddr_in sockaddr;        //sockaddr for server
  
  // Tracker info
  string infoHash;                    // info hash
  Torrent metaData;

  // Book keeping
  vector<void*> peers;            // Peers in torrent
  fstream saveFile_fd;                // File descriptor
  time_t start_time;                  // start time
  bool isComplete;                    // donwload complete
  char *piecesBitVector;      // piece bit vector
  size_t bitVectorSize;
  // ref objects
  //FileManager *m_filemanager;
  //Reactor *reactor;
  

};
#endif 
