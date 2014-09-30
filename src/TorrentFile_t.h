/*
 * TorrentFile_t.h
 *
 *  Created on: Sep 22, 2014
 *      Author: rkhapare
 */

#ifndef TORRENTFILE_T_H_
#define TORRENTFILE_T_H_

#include "BencodeDictionary_t.h"

#include <string>
#include <vector>
using namespace std;

class TorrentFile_t
{
 private:
  size_t length;
  vector<string> paths;

 public:
  size_t getLength();
  void setLength(unsigned int);
  void setLength(BencodeDictionary_t *);

  vector<string> getPaths();
  void setPaths(BencodeDictionary_t *);
  void addPath(string);
  string pathAt(unsigned int);
};

#endif /* TORRENTFILE_T_H_ */
