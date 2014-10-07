/*
 * FileHandler.hpp
 *
 *  Created on: Oct 06, 2014
 *      Author: rkhapare
 */

#ifndef FILEHANDLER_HPP_
#define FILEHANDLER_HPP_

#include "Torrent.hpp"

#include <string>
#include <vector>
#include <fstream>
using namespace std;

class FileHandler
{
 private:
  Torrent& torrent;
  vector<string> saveFiles;
  vector<fstream> fileStreams;

 public:
  FileHandler(Torrent &, vector<string>);
  ~FileHandler();

  bool openOrCreateFile(string, size_t, fstream&);
  bool readPiece(size_t, string &, size_t &);
  //void writeBlock(size_t, size_t, size_t, string &);
  bool isValidPiece(size_t);
  bool checkHashes(unsigned char *, unsigned char *, size_t);
};

#endif /* FILEHANDLER_HPP_ */
