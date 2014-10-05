#include <vector>

#include "TorrentCtx.hpp"
#include "Logger.hpp"

#include "Torrent.hpp"
#include "TorrentFile.hpp"
#include <openssl/sha.h>

using namespace std;

TorrentCtx::TorrentCtx(Reactor *r){
  reactor = r;
}

void TorrentCtx::init(bt_args_t *args){
  
  char id[ID_SIZE];
  char infoHash[20];
  Torrent torrent;
  vector<TorrentFile> files;
  string infoDict;

  // error handling
  if(args == NULL ){
    LOG(ERROR, "Invalid arguments ");
    exit(EXIT_FAILURE);
  }

  // copy data from arguments
  saveFile = args->save_file;
  torrentFile = args->torrent_file;
  sockaddr = reactor->getSocketAddr();

  //if id is not provided calculate it.
  if(strlen(args->id) == 0){
    calc_id_sockaddr(&sockaddr, id);
    peerId = string(id);
  }else{
    peerId = string(args->id);
  }
  
  LOG(INFO, "Client Id :" + peerId);

  // parse torrent file and get meta info.
  torrent = Torrent::decode(string(torrentFile));

  // check for multiple file
  files = torrent.getFiles();
  if(files.size() > 1){
    LOG(ERROR,"This version of Bit Torrent does not support multiple file download");
    exit(EXIT_FAILURE);
  }

  announce = torrent.getAnnounce();
  torrentName = torrent.getName();
  pieceLength = torrent.getPieceLength();
  pieceHashes = torrent.getPieceHashes();
  numPieces = pieceHashes.size();
  infoDict = torrent.getInfoDictionary();

  for(vector<TorrentFile>::iterator it = files.begin(); it != files.end(); ++it ){
    TorrentFile f = *it;
    fileLength = f.getLength();
  }
  
  LOG(INFO, "Annoumce : "+ announce);
  LOG(INFO, "Torrent Name : "+ torrentName);
  LOG(INFO, "piece Length : "+ to_string(pieceLength));
  LOG(INFO, "Number of pieces : "+ to_string(numPieces));
  LOG(INFO, "File Length : "+ to_string(fileLength));

  // calculate the infohash
 SHA1((unsigned char *)infoDict.c_str(), infoDict.length(), (unsigned char *)infoHash);

  
  // Check how many pieces we have ? Compute hash over them and verify. After that  Build the bitvector. 
  // Check isComplete ?
  

  
}

void TorrentCtx::contact_tracker(bt_args_t * bt_args){
  
}
