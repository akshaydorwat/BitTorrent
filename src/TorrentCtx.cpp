
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

using namespace std;

/*TorrentCtx::TorrentCtx(Reactor *r){
  reactor = r;
  }*/

void TorrentCtx::init(bt_args_t *args){
  
  char id[ID_SIZE];
  char md[20];
  vector<TorrentFile> files;
  string infoDict;
  string filename;
  ifstream inp;
  ofstream out;
  
  // error handling
  if(args == NULL ){
    LOG(ERROR, "Invalid arguments ");
    exit(EXIT_FAILURE);
  }

  // copy data from arguments
  saveFile = args->save_file;
  torrentFile = args->torrent_file;
  sockaddr = Reactor::getInstance()->getSocketAddr();

  //if id is not provided calculate it.
  if(strlen(args->id) == 0){
    calc_id_sockaddr(&sockaddr, id);
    peerId = string(id);
  }else{
    peerId = string(args->id);
  }
  
  LOG(INFO, "Client Id("+to_string(peerId.length())+"):" + peerId);

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

  if(!isComplete){
    for (std::map<unsigned char*, void*>::iterator it=peers.begin(); it!=peers.end(); ++it){
      Peer *p = (Peer*) it->second;		
      unsigned char *ptr = p->getId();
      printf("\nId:");
      for(int i=0;i<20;i++){
	printf("%2x", ptr[i]);
      }
      printf("\n");
      //p->startConnection();
      }
    }
  // Check isComplete ?
  
}

void TorrentCtx::contact_tracker(bt_args_t * bt_args){

  int i;
  peer_t *p;
  LOG(INFO, "Number of peers in the list :"+ to_string(bt_args->n_peers));
  for(i=0; i< bt_args->n_peers; i++){
    p = bt_args->peers[i];
    if(p != NULL){
      Peer *peer_obj = new Peer(this, p);
      peers[p->id] = peer_obj;
    }
  }
}
