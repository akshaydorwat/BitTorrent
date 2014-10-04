#ifndef TORRENT_CTX_HPP
#define TORRENT_CTX_HPP

#include <map>
#include <vector>
#include <string>

#include "Peer.hpp"
#include "bt_lib.h"
#include "Reactor.hpp"
#include <time.h>

using namespace std;

class TorrentCtx{

public:
  TorrentCtx(Reactor *r);

  // initialise torrent context
  void init(bt_args_t *args);
  
  // contact tracker and load the peers list
  void contact_tracker(bt_args_t * bt_args);
  
private:
  //command line arguments
  string peerId;                      // peer id
  string saveFile;                    //the filename to save to
  string torrentFile;                 //torrent file name
  struct sockaddr_in sockaddr;        //sockaddr for server
  
  // Tracker info
  string infoHash;                    // info hash
  string announce;                    // announce url
  string torrentName;                 // torrent 
  int64_t pieceLength;                // piece length
  int64_t fileLength;                 // total file len
  size_t numPieces;                   // numer of pieces
  map<string,Peer*> peers;            // Peers in torrent
  vector<string> pieceHashes;          // pieces hash

  // Book keeping
  time_t start_time;                 // start time
  bool isComplete;                   // donwload complete
  vector<bool> *piecesBitVector;     // piece bit vector
  
  // ref objects
  //FileManager *m_filemanager;
  Reactor *reactor;
  

};
#endif 
