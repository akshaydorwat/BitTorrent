/*
 * TorrentPiece_t.h
 *
 *  Created on: Oct 04, 2014
 *      Author: rkhapare
 */

#ifndef TORRENTPIECE_T_H_
#define TORRENTPIECE_T_H_

#include <string>
#include <vector>
using namespace std;

class TorrentPiece_t
{
 private:
  size_t length;
  //string data;
  //vector<int> blockAvailable;
  //vector<TorrentPieceBlock_t> blocks;
  string hash;

 public:
  TorrentPiece_t(size_t, string);

  size_t getLength();
  void setLength(size_t);

  //void setData(string);
  //string getData();
  //bool isComplete();

  void setHash(string);
  string getHash();
  //bool isValid();
};

#endif /* TORRENTPIECE_T_H_ */
