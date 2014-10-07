
#include <fstream>
#include <openssl/sha.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "TorrentCtx.hpp"
#include "Logger.hpp"
#include "Peer.hpp"
#include "Reactor.hpp"
#include <stdio.h>
#include <thread>
#include <stdio.h>
#include "bt_lib.h"
#include <string.h>

using namespace std;

/*TorrentCtx::TorrentCtx(Reactor *r){
  reactor = r;
  }*/

void TorrentCtx::init(bt_args_t *args){
  
  char md[20];
  vector<TorrentFile> files;
  string infoDict;
  string filename;
  ifstream inp;
  ofstream out;
  unsigned short port;

  // error handling
  if(args == NULL ){
    LOG(ERROR, "Invalid arguments ");
    exit(EXIT_FAILURE);
  }

  // copy data from arguments
  saveFile = args->save_file;
  torrentFile = args->torrent_file;
  sockaddr = Reactor::getInstance()->getSocketAddr();
  port = Reactor::getInstance()->getPortUsed();
  //if id is not provided calculate it.
  if(strlen(args->id) == 0){
    calc_id(args->ip, port, (char *)peerId);
  }else{
    bcopy(args->id, peerId, ID_SIZE);
  }
  
  LOG(INFO, "Client Id("+to_string(strlen((const char*)peerId))+"):");
  print_peer_id(peerId);

  // parse torrent file and get meta info.
  metaData = Torrent::decode(string(torrentFile));

  // check for multiple file
  files = metaData.getFiles();
  if(files.size() > 1){
    LOG(ERROR,"This version of Bit Torrent does not support multiple file download");
    exit(EXIT_FAILURE);
  }

  // calculate the infohash
  infoDict = metaData.getInfoDictionary();
  SHA1((unsigned char *)infoDict.c_str(), infoDict.length(), (unsigned char *)md);
  infoHash = string(md);
  LOG(INFO, "Calculated Info hash successfully" );
  
  // check if file exist
  filename = saveFile +"/" + metaData.getName();
  saveFile_fd.open(filename.c_str(), std::fstream::in | std::fstream::out);
  if(!saveFile_fd.is_open()){
    saveFile_fd.close();
    saveFile_fd.open(filename.c_str(), std::fstream::out);
    if(!saveFile_fd.is_open()){
      LOG(ERROR, "Unable to open file");
      exit(EXIT_FAILURE);
    }
    saveFile_fd.close();
    saveFile_fd.open(filename.c_str(), std::fstream::in | std::fstream::out);
    if(!saveFile_fd.is_open()){
      LOG(ERROR, "Unable to open file");
      exit(EXIT_FAILURE);
    }
    LOG(INFO, "File :" + filename + " opened sucessfully");
  } 
  
  // Load pieces from the file 
  // Check how many pieces we have ? Compute hash over them and verify. After that  Build the bitvector. 
  //loadPieces();
  contact_tracker(args);
  // If download is not complete start connection to seeder and intiate handshake
  isComplete = true;
  if(!isComplete){
    for (vector< void*>::iterator it=peers.begin(); it!=peers.end(); ++it){
      Peer *p = (Peer*) *it;		
      std::thread t(&Peer::startConnection, p);
      // TODO: Not sure  about detaching but it works i do
      t.detach();
    }
  }
}

void TorrentCtx::contact_tracker(bt_args_t * bt_args){
  
  int i;
  peer_t *p;
  LOG(INFO, "Number of peers in the list :"+ to_string(bt_args->n_peers));
  for(i=0; i< bt_args->n_peers; i++){
    p = bt_args->peers[i];
    print_peer(p);
    if(p != NULL){
      Peer *peer_obj = new Peer(this, p);
      peers.push_back(peer_obj);
    }
  }
}
  
void* TorrentCtx::getPeer(unsigned char *id){
  
  LOG(DEBUG, "Peer to find :");
  print_peer_id(id);

  for (vector< void*>::iterator it=peers.begin(); it!=peers.end(); ++it){
    Peer *p = (Peer*) *it;		
    if(p != NULL){
      if(memcmp(p->getId(), id, ID_SIZE) == 0){
	LOG(INFO, "Peer found for connection!");
	return (void*)p;
      }
    }else{
      LOG(WARNING, "NULL pointer in peer list");
    }
  }
  return NULL;
}
