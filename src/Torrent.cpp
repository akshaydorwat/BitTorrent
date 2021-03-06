
/*
 * Torrent.cpp
 *
 *  Created on: Sep 22, 2014
 *      Author: rkhapare
 */

#include "Torrent.hpp"
#include "TorrentFile.hpp"
//#include "TorrentPiece_t.h"
#include "BencodeDecoder.h"
#include "Bencode_t.h"
#include "BencodeDictionary_t.h"
#include "BencodeList_t.h"
#include "BencodeString_t.h"
#include "BencodeInteger_t.h"
#include "Logger.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <ctime>
#include <exception>
#include <stdexcept>
#include <cassert>
#include <stdlib.h>
#include <fstream>

#include <stdio.h>
#include <stdlib.h>
using namespace std;

Torrent Torrent::decode(string torrentString)
{
  string ending = ".torrent";
  if (torrentString.length() > ending.length()
      && 0 == torrentString.compare(
				    torrentString.length() - ending.length(),
				    ending.length(), ending))
    {
      long int size;
      char * memblock;

      ifstream file(torrentString.c_str(), ios::in | ios::binary | ios::ate);
      if (file.is_open())
	{
	  size = file.tellg();
	  //			cout << "Reading .torrent file of size " << size << " byte(s)." << endl;
	  memblock = new char[size];
	  file.seekg(0, ios::beg);
	  file.read(memblock, size);
	  file.close();

	  string temp(memblock, size);
	  torrentString = temp;
	  delete[] memblock;
	}
      else
	throw invalid_argument(
			       "Failed to access .torrent file. Please check the entered path and/or access rights.");
    }

  //	cout << "Extracting torrent meta-info from [" << torrentString << "]" << endl << endl;// of size " << torrentString.size() << " characters." << endl << endl;

  size_t startIdx = 0;
  Bencode_t *bencode_t = BencodeDecoder::decode(torrentString, &startIdx);
  assert(bencode_t);

  //cout << bencode_t->display() << endl;

  Torrent newTorrent;
  char buffer[33];	
  try
    {
      //cout << "TORRENT PROPERTY\t:\t\tVALUE(S)" << endl << endl;
      LOG (INFO, "TORRENT PROPERTY\t:\t\tVALUE(S)");

      newTorrent.setName(bencode_t);
      //cout << "Name\t\t\t:\t\t" << newTorrent.getName() << endl << endl;
      LOG (INFO, "Name\t\t:\t\t" + newTorrent.getName());

      try
	{
	  newTorrent.setCreator(bencode_t);
	  //cout << "Creator\t\t\t:\t\t" << newTorrent.getCreator() << endl
	  //		<< endl;
	  LOG (INFO, "Creator\t\t\t:\t\t" + newTorrent.getCreator());
	}
      catch (const exception& e)
	{
	}

      try
	{
	  newTorrent.setCreatedOn(bencode_t);
	  //cout << "Creation Date\t\t:\t\t" << newTorrent.showCreatedOn()
	  //		<< endl;
	  LOG (INFO, "Creation Date\t:\t\t" + newTorrent.showCreatedOn());
	}
      catch (const exception& e)
	{
	}

      try
	{
	  newTorrent.setAnnounce(bencode_t);
	  //cout << "Announce\t\t:\t\t" << newTorrent.getAnnounce() << endl
	  //		<< endl;
	  LOG (INFO, "Announce\t\t:\t\t" + newTorrent.getAnnounce());
	}
      catch (const exception& e)
	{
	}

      try
	{
	  newTorrent.setAnnounceList(bencode_t);
	  vector<string> announceList = newTorrent.getAnnounceList();
	  //cout << "Announce List (" << announceList.size() << ")\t:\t\t";
	  LOG (INFO, "Announce List\t:\t\t");
	  for (size_t i = 0; i < announceList.size(); i++)
	    {
	      //cout << announceList.at(i);
	      LOG (INFO, announceList.at(i));
	      if (i < announceList.size() - 1)
		LOG (INFO, ",\n\t\t\t\t\t");
	      //cout << ",\n\t\t\t\t\t";
	    }
	  //cout << endl << endl;
	}
      catch (const exception& e)
	{
	}

      try
	{
	  newTorrent.setComment(bencode_t);
	  //cout << "Comment\t\t\t:\t\t" << newTorrent.getComment() << endl
	  //		<< endl;
	  LOG (INFO, "Comment\t\t\t:\t\t" + newTorrent.getComment());
	}
      catch (const exception& e)
	{
	}

      try
	{
	  newTorrent.setEncoding(bencode_t);
	  //cout << "Encoding\t\t:\t\t" << newTorrent.getEncoding() << endl
	  //		<< endl;
	  LOG (INFO, "Encoding\t\t:\t\t" + newTorrent.getEncoding());
	}
      catch (const exception& e)
	{
	}

      newTorrent.setPieceLength(bencode_t);
      //cout << "Piece Length\t\t:\t\t" << newTorrent.getPieceLength() << endl
      //		<< endl;
      sprintf(buffer, "%lu", newTorrent.getPieceLength());
      LOG (INFO, "Piece Length\t:\t\t" + string(buffer));

      BencodeDictionary_t* torrentDictionary =
	dynamic_cast<BencodeDictionary_t*>(bencode_t);
      BencodeDictionary_t *infoDictionary =
	dynamic_cast<BencodeDictionary_t*>(torrentDictionary->at("info"));
	//newTorrent.infoDictionary = infoDictionary->clone();
	newTorrent.setInfoDictionary(infoDictionary);
	//LOG (INFO, "Info Dictionary \t:\t\t" + newTorrent.getInfoDictionary());

      BencodeList_t *filesList =
	dynamic_cast<BencodeList_t*>(infoDictionary->at("files"));
      if (filesList) // multiple files torrent
	{
	  /* NOTE:files is a list of dictionaries. eg. in bencoding: ld6:lengthi1024e4path:l8:filenameeee */
	  for (size_t i = 0; i < filesList->size(); i++)
	    {
	      BencodeDictionary_t *filesDictionary =
		dynamic_cast<BencodeDictionary_t*>(filesList->at(i));
	      newTorrent.addFile(filesDictionary);
	    }
	}
      else // single file torrent
	{
	  newTorrent.addFile(infoDictionary);
	}
      vector<TorrentFile> files = newTorrent.getFiles();
      //cout << "Files (" << files.size() << ")\t\t:\t\t";
      LOG (INFO, "Files\t\t:\t\t");
      for (size_t i = 0; i < files.size(); i++)
	{
	  //cout << files.at(i).pathAt(0) << "\t(" << files.at(i).getLength() << ")";
	  sprintf(buffer, "%lu", files.at(i).getLength());
	  LOG (INFO, files.at(i).pathAt(0) + "\t(" + string(buffer) + ")");
	  if (i < files.size() - 1)
	    LOG (INFO, ",\n\t\t\t\t\t");
	  //cout << ",\n\t\t\t\t\t";
	}
      //cout << endl << endl;

      newTorrent.setPieceHashes(bencode_t);
      vector<string> pieceHashes = newTorrent.getPieceHashes();
      //cout << "Piece Hashes (" << pieceHashes.size() << ")\t:\t\t";
      sprintf(buffer, "%lu", pieceHashes.size());
      string buf("Piece Hashes (");
      buf += string(buffer);
      buf += ")\t:\t\t";
      //LOG (INFO, "Piece Hashes (" + string(buffer) + ")\t:\t\t");
      for (size_t i = 0; i < pieceHashes.size(); i++)
	{
	  //cout << pieceHashes.at(i);
	  //LOG (INFO, pieceHashes.at(i));
	  buf += pieceHashes.at(i);
	  if (i < pieceHashes.size() - 1)
	    buf += ",\n\t\t\t\t\t";
	  //LOG (INFO, ",\n\t\t\t\t\t");
	  //cout << ",\n\t\t\t\t\t";
	}
      //LOG (INFO, buf);
      //cout << endl << endl;
    }
  catch (const exception& e)
    {
      //cout << "Failed to parse torrent !!! Invalid Format !!!\n" << e.what() << endl;
      LOG (ERROR, "Failed to parse torrent !!! Invalid Format !!!");
    }

  return newTorrent;
}

/////////////////////////////////////////////////////////////////////////////////
void Torrent::setInfoDictionary(BencodeDictionary_t *infoDictionary_t)
{
	if (infoDictionary_t)
	{
		infoDictionary += infoDictionary_t->encode();
	}
	else
    	{
      		throw invalid_argument("Failed to find \"info\" dictionary !!!");
    	}
}

string Torrent::getInfoDictionary()
{
	return infoDictionary;
}

/////////////////////////////////////////////////////////////////////////////////
string Torrent::getAnnounce()
{
  return announce;
}

void Torrent::setAnnounce(string str)
{
  announce = str;
}

void Torrent::setAnnounce(Bencode_t *bencodeTorrent)
{
  BencodeDictionary_t* torrentDictionary =
    dynamic_cast<BencodeDictionary_t*>(bencodeTorrent);
  BencodeString_t *bencodeString_t =
    dynamic_cast<BencodeString_t*>(torrentDictionary->at("announce"));
  if (bencodeString_t)
    {
      announce = bencodeString_t->get();
    }
  else
    {
      throw invalid_argument("Failed to find \"announce\" object !!!");
    }
}

/////////////////////////////////////////////////////////////////////////////////
vector<string> Torrent::getAnnounceList()
{
  return announceList;
}

void Torrent::setAnnounceList(vector<string> list)
{
  announceList = list;
}

void Torrent::setAnnounceList(Bencode_t *bencodeTorrent)
{
  BencodeDictionary_t* torrentDictionary =
    dynamic_cast<BencodeDictionary_t*>(bencodeTorrent);
  BencodeList_t *bencodeList_t =
    dynamic_cast<BencodeList_t*>(torrentDictionary->at("announce-list"));

  if (bencodeList_t)
    {
      for (size_t i = 0; i < bencodeList_t->size(); i++)
	{
	  BencodeList_t *bencodeList_ts =
	    dynamic_cast<BencodeList_t*>(bencodeList_t->at(i));

	  /* NOTE:announce-list is a list of list of strings. eg. in bencoding: ll5:hello3:how3:are3:youee
	   * Currently extracting only the first string in the list of strings. */
	  BencodeString_t *bencodeString_t =
	    dynamic_cast<BencodeString_t*>(bencodeList_ts->at(0));

	  announceList.push_back(bencodeString_t->get());
	}
    }
  else
    {
      throw invalid_argument("Failed to find \"announce-list\" object !!!");
    }
}

void Torrent::addToAnnounceList(string str)
{
  announceList.push_back(str);
}

/////////////////////////////////////////////////////////////////////////////////
string Torrent::getComment()
{
  return comment;
}

void Torrent::setComment(string str)
{
  comment = str;
}

void Torrent::setComment(Bencode_t *bencodeTorrent)
{
  BencodeDictionary_t* torrentDictionary =
    dynamic_cast<BencodeDictionary_t*>(bencodeTorrent);
  BencodeString_t *bencodeString_t =
    dynamic_cast<BencodeString_t*>(torrentDictionary->at("comment"));
  if (bencodeString_t)
    {
      comment = bencodeString_t->get();
    }
  else
    {
      throw invalid_argument("Failed to find \"comment\" object !!!");
    }
}

/////////////////////////////////////////////////////////////////////////////////
string Torrent::getCreator()
{
  return creator;
}

void Torrent::setCreator(string str)
{
  creator = str;
}

void Torrent::setCreator(Bencode_t *bencodeTorrent)
{
  BencodeDictionary_t* torrentDictionary =
    dynamic_cast<BencodeDictionary_t*>(bencodeTorrent);
  BencodeString_t *bencodeString_t =
    dynamic_cast<BencodeString_t*>(torrentDictionary->at("created by"));

  if (bencodeString_t)
    {
      creator = bencodeString_t->get();
    }
  else
    {
      throw invalid_argument("Failed to find \"created by\" object !!!");
    }
}

/////////////////////////////////////////////////////////////////////////////////
time_t Torrent::getCreatedOn()
{
  return createdOn;
}

string Torrent::showCreatedOn()
{
  return ctime(&createdOn);
}

void Torrent::setCreatedOn(Bencode_t *bencodeTorrent)
{
  BencodeDictionary_t* torrentDictionary =
    dynamic_cast<BencodeDictionary_t*>(bencodeTorrent);
  BencodeInteger_t *bencodeInteger_t =
    dynamic_cast<BencodeInteger_t*>(torrentDictionary->at(
							  "creation date"));

  if (bencodeInteger_t)
    {
      createdOn = static_cast<time_t>(bencodeInteger_t->get() / 1000);
    }
  else
    {
      throw invalid_argument("Failed to find \"creation date\" object !!!");
    }
}

/////////////////////////////////////////////////////////////////////////////////
string Torrent::getEncoding()
{
  return encoding;
}

void Torrent::setEncoding(string str)
{
  encoding = str;
}

void Torrent::setEncoding(Bencode_t *bencodeTorrent)
{
  BencodeDictionary_t* torrentDictionary =
    dynamic_cast<BencodeDictionary_t*>(bencodeTorrent);
  BencodeString_t *bencodeString_t =
    dynamic_cast<BencodeString_t*>(torrentDictionary->at("encoding"));

  if (bencodeString_t)
    {
      encoding = bencodeString_t->get();
    }
  else
    {
      throw invalid_argument("Failed to find \"encoding\" object !!!");
    }
}

/////////////////////////////////////////////////////////////////////////////////
string Torrent::getName()
{
  return name;
}

void Torrent::setName(string str)
{
  name = str;
}

void Torrent::setName(Bencode_t *bencodeTorrent)
{
  BencodeDictionary_t* torrentDictionary =
    dynamic_cast<BencodeDictionary_t*>(bencodeTorrent);
  BencodeDictionary_t *bencodeDictionary_t =
    dynamic_cast<BencodeDictionary_t*>(torrentDictionary->at("info"));
  BencodeString_t *bencodeString_t =
    dynamic_cast<BencodeString_t*>(bencodeDictionary_t->at("name"));

  if (bencodeString_t)
    {
      name = bencodeString_t->get();
    }
  else
    {
      throw invalid_argument("Failed to find \"name\" object !!!");
    }
}

/////////////////////////////////////////////////////////////////////////////////
size_t Torrent::getPieceLength()
{
  return pieceLength;
}

void Torrent::setPieceLength(size_t size)
{
  pieceLength = size;
}

void Torrent::setPieceLength(Bencode_t *bencodeTorrent)
{
  BencodeDictionary_t* torrentDictionary =
    dynamic_cast<BencodeDictionary_t*>(bencodeTorrent);
  BencodeDictionary_t *bencodeDictionary_t =
    dynamic_cast<BencodeDictionary_t*>(torrentDictionary->at("info"));
  BencodeInteger_t *bencodeInteger_t =
    dynamic_cast<BencodeInteger_t*>(bencodeDictionary_t->at(
							    "piece length"));

  if (bencodeInteger_t)
    {
      pieceLength = bencodeInteger_t->get();
    }
  else
    {
      throw invalid_argument("Failed to find \"piece length\" object !!!");
    }
}

/////////////////////////////////////////////////////////////////////////////////
/*void Torrent::piece(string pieceHash)
{
	size_t pieceLen = 1 + pieces.size();
	pieceLen *= (size_t) pieceLength;
	if (length < pieceLen)	// if not last piece
		pieceLen -= (int) pieces.size() * pieceLength;
	else			// else, adjust for last piece
		pieceLen = length - pieceLen;

	TorrentPiece_t newPiece (pieceLen, pieceHash);
	pieces.push_back(newPiece);

	if (pieceHash.size() < sizeof(int) * pieces.size())
		pieceHash.push_back(0);
}

TorrentPiece_t Torrent::piece(int idx)
{
	assert (idx < (int) pieces.size());
	return pieces[idx];
}*/

vector<string> Torrent::getPieceHashes()
{
	/*vector<string> pieceHashes;
	for (size_t i=0; i < pieces.size(); i++)
	{
		pieceHashes.push_back(pieces[i].getHash());
	}*/
  	return pieceHashes;
}

void Torrent::setPieceHashes(Bencode_t *bencodeTorrent)
{
  BencodeDictionary_t* torrentDictionary =
    dynamic_cast<BencodeDictionary_t*>(bencodeTorrent);
  BencodeDictionary_t *bencodeDictionary_t =
    dynamic_cast<BencodeDictionary_t*>(torrentDictionary->at("info"));
  BencodeString_t *bencodeString_t =
    dynamic_cast<BencodeString_t*>(bencodeDictionary_t->at("pieces"));

  if (bencodeString_t)
    {
      string temp = bencodeString_t->get();
      for (unsigned int i = 0; i < temp.size(); i += 20)
	{
	  pieceHashes.push_back(temp.substr(i, 20));
	  //piece(temp.substr(i, 20));
	}
    }
  else
    {
      throw invalid_argument("Failed to find \"pieces\" object !!!");
    }
}

void Torrent::addPieceHash(string str)
{
  pieceHashes.push_back(str);
}

string Torrent::pieceHashAt(size_t i)
{
  assert (pieceHashes.size() > i);
  return pieceHashes[i];
}

/////////////////////////////////////////////////////////////////////////////////
vector<TorrentFile> Torrent::getFiles()
{
  return files;
}

TorrentFile Torrent::getFileAt(size_t i)
{
  assert(i >= 0 && i < files.size());
  return files[i];
}

void Torrent::addFile(TorrentFile file)
{
  files.push_back(file);
}

void Torrent::addFile(BencodeDictionary_t* filesDictionary)
{
  TorrentFile torrentFile_t;
  torrentFile_t.setLength(filesDictionary);
  try
    {
      torrentFile_t.setPaths(filesDictionary);
    }
  catch (const invalid_argument& e)
    {
      torrentFile_t.addPath(name);
    }
  files.push_back(torrentFile_t);
}

/////////////////////////////////////////////////////////////////////////////////
