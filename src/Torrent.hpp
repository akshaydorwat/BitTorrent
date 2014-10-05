/*
 * Torrent.hpp
 *
 *  Created on: Sep 22, 2014
 *      Author: rkhapare
 */

#ifndef TORRENT_HPP_
#define TORRENT_HPP_

#include "TorrentFile.hpp"
#include "Bencode_t.h"

#include <string>
#include <vector>
#include <ctime>
using namespace std;

class Torrent
{
 private:
  string announce;
  vector<string> announceList;
  string comment;
  string creator;
  time_t createdOn;
  string encoding;
  vector<TorrentFile> files;
  string name;
  size_t pieceLength;
  vector<string> pieceHashes;
  string infoDictionary;

 public:
  //static string encode();

  static Torrent decode(string);

  string getAnnounce();
  void setAnnounce(string);
  void setAnnounce(Bencode_t *);

  vector<string> getAnnounceList();
  void setAnnounceList(vector<string>);
  void setAnnounceList(Bencode_t *);
  void addToAnnounceList(string);

  string getComment();
  void setComment(string);
  void setComment(Bencode_t *);

  string getCreator();
  void setCreator(string);
  void setCreator(Bencode_t *);

  time_t getCreatedOn();
  string showCreatedOn();
  void setCreatedOn(Bencode_t *);

  string getEncoding();
  void setEncoding(string);
  void setEncoding(Bencode_t *);

  vector<TorrentFile> getFiles();
  TorrentFile getFileAt(int);
  void addFile(TorrentFile);
  void addFile(BencodeDictionary_t*);

  string getName();
  void setName(string);
  void setName(Bencode_t *);

  size_t getPieceLength();
  void setPieceLength(size_t);
  void setPieceLength(Bencode_t *);

  vector<string> getPieceHashes();
  void setPieceHashes(Bencode_t *);
  void addPieceHash(string);
  string pieceHashAt(int);
  //void piece(string);
  //TorrentPiece_t piece(int);
  //bool isValidPiece(size_t);

  string getInfoDictionary();
  void setInfoDictionary(BencodeDictionary_t *); 
};

#endif /* TORRENT_HPP_ */
