
#include <fstream>
#include <thread>
#include <iostream>
#include <openssl/sha.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include "bt_lib.h"
#include "TorrentCtx.hpp"
#include "Logger.hpp"
#include "Peer.hpp"
#include "Reactor.hpp"
#include "PieceRequestor.hpp"
#include "PieceProcessor.hpp"

using namespace std;

TorrentCtx::TorrentCtx(){
  pieceProcessor = NULL;
  pieceRequestor = NULL;
  requestProcessor = NULL;
  piecesBitVector = NULL;
}

TorrentCtx::~TorrentCtx()
{
 
  if(pieceProcessor)
    delete pieceProcessor;

  if(pieceRequestor)
  {
	//LOG (DEBUG, "TorrentCtx : Stopping Piece Requestor.");
    pieceRequestor->stopPieceRequestor();
    pieceRequestorThread.join();
    delete pieceRequestor;
  }
  
  if(requestProcessor){ 
    delete requestProcessor;
  }
  
  for (size_t i=0; i<pieces.size(); i++)
    delete pieces[i];

  delete fileMgr;
  
  if(piecesBitVector){
    delete piecesBitVector;
  }

  for (vector< void*>::iterator it=peers.begin(); it!=peers.end(); ++it){
    Peer *p = (Peer*) *it;		
    delete p;
  }
}

void TorrentCtx::init(bt_args_t *args){
  
  char md[20];
  //vector<TorrentFile> files;
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
  //files = metaData.getFiles();
  //if(files.size() > 1){
  if (metaData.numOfFiles() > 1) {
    LOG(ERROR,"This version of Bit Torrent does not support multiple file download");
    exit(EXIT_FAILURE);
  }

  // calculate the infohash
  infoDict = metaData.getInfoDictionary();
  SHA1((unsigned char *)infoDict.c_str(), infoDict.length(), (unsigned char *)md);
  infoHash = string(md);
  LOG(INFO, "Calculated Info hash successfully" );

  // Initialise the bit vector
  initBitVecor();

  // check if file exist
  filename = saveFile +"/" + metaData.getName();
  LOG (INFO, "Target file " + filename);
  vector<string> saveFiles;
  saveFiles.push_back(filename);
  
  fileMgr = new FileHandler (metaData, saveFiles);
  LOG (INFO, "New File Handler initiated."); 

  // Load pieces from the file 
  // Check how many pieces we have ? Compute hash over them and verify. After that  Build the bitvector. 
  
  loadPieceStatus();
  LOG (INFO, "Loaded Piece availability.");

  // load Peers
  contact_tracker(args);
  
  // depending upon the role start managers
  if(!complete){                                                              // LEECHER
    // try to start connections to seeder
    for (vector< void*>::iterator it=peers.begin(); it!=peers.end(); ++it){
      Peer *p = (Peer*) *it;		
      std::thread t(&Peer::startConnection, p);
      // TODO: Not sure  about detaching but it works.
      t.detach();
    }
   
    LOG(DEBUG,"Starting REQUEST maker");
    // start piece requestor
    pieceRequestor = new PieceRequestor(pieces, peers);
    pieceRequestorThread = thread (&PieceRequestor::startPieceRequestor, pieceRequestor);
    //t.detach();

    LOG(DEBUG, "starting PIECE processor");
    // start piece processor
    pieceProcessor = new PieceProcessor(pieces, pieceRequestor);
    
  }else{                                                                      // SEEDER
    LOG(DEBUG, "starting REQUEST processor");
    // start request processor
    requestProcessor = new RequestProcessor(&pieces);
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


void TorrentCtx::loadPieceStatus()
{
  //cout << "Loading " << metaData.numOfPieces() << " pieces. Length of Pieces " << metaData.getPieceLength() << endl;
  bool AllPiecesCompleteFlag = true; 
  for (size_t i=0; i < metaData.numOfPieces(); i++)
    {
      size_t pieceLength = i+1 < metaData.numOfPieces() ? metaData.getPieceLength() : metaData.getFiles().back().getLength() - (i * metaData.getPieceLength());
      //cout << "Piece#" << i << " size = " << pieceLength << " bytes" << endl;
      pieces.push_back(new Piece(i, i * metaData.getPieceLength(), pieceLength, metaData.pieceHashAt(i), fileMgr));
      //LOG (DEBUG, "Checking validity of piece.");
      string pieceData;
      bool pieceAvailable = fileMgr->readIfValidPiece(i, pieceData);
      if (pieceAvailable){
	pieces.back()->setValid();
	pieces.back()->setAvailable();
	setbit((size_t)i);
	LOG(DEBUG, "BIT[" + to_string(i) + "] = true");
      }else{
	AllPiecesCompleteFlag = false;
	LOG(DEBUG, "BIT[" + to_string(i) + "] = false");
      }
    }
  // If all the pieces are available then set isComplete flag
  if(AllPiecesCompleteFlag){
    LOG(INFO, "This client will act as SEEDER");
    complete = true;
  }else{
    LOG(INFO, "This client will act as LEECHER");
  }

}

void TorrentCtx::initBitVecor(){
  size_t mbyte;
  size_t mbit;
  size_t size =  getNumOfPieces();

  if( size > 0){
    mbyte = size / 8;
    mbit  = size % 8;

    if(mbit > 0) {
      mbyte++;
    }
    bitVectorSize = mbyte;
    piecesBitVector = new char[bitVectorSize];
    memset(piecesBitVector, 0, mbyte*sizeof(char));
  }else{
    LOG(ERROR, "Invalid number of pieces ");
    exit(EXIT_FAILURE);
  }
}

char* TorrentCtx::getPiecesBitVector()
{
  for (size_t i=0; i<pieces.size(); i++){
    if (getbit(i) == 0 && pieces[i]->isValid())
      setbit(i);
  }
  return piecesBitVector;
}

void TorrentCtx::setbit( size_t b) {
  size_t mbyte, mbit;
 
  mbyte = b / 8;
  mbit = b % 8;

  if(mbyte > bitVectorSize){
    LOG(ERROR, "Can not set bit");
  }
  piecesBitVector[mbyte] |= (0x80 >> mbit);
}

int TorrentCtx::getbit( size_t b) {
  size_t mbyte, mbit;

  mbyte = b / 8;
  mbit = b % 8;

  if((size_t)mbyte > bitVectorSize){
    LOG(ERROR, "Can not get bit");
    exit(EXIT_FAILURE);
  }

  return( ( (piecesBitVector[mbyte] << mbit) & 0x80 ) >> 7);
}

