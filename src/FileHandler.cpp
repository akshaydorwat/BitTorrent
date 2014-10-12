/*
 * FileHandler.cpp
 *
 *  Created on: Oct 06, 2014
 *      Author: rkhapare
 */

#include "Torrent.hpp"
#include "FileHandler.hpp"
#include "Piece.hpp"
#include "Logger.hpp"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <openssl/sha.h>
#include <mutex>
//#include <ctime>
//#include <exception>
//#include <stdexcept>
//#include <cassert>
//#include <stdlib.h>
using namespace std;

FileHandler::FileHandler(Torrent& torrent, vector<string>& saveFiles)
  :torrent(torrent)
  ,saveFiles(saveFiles)
{
  //LOG (INFO, "Creating new FileHandler.");
  size_t numOfFiles = 1; // torrent.numOfFiles();
  vector<TorrentFile> torrentFiles = torrent.getFiles();
  //LOG (INFO, "Extracted TorrentFiles.");
  if (saveFiles.size() < numOfFiles)
    {	
      for(size_t i=saveFiles.size(); i < numOfFiles; i++)
	saveFiles.push_back(torrentFiles[i].pathAt(0));
    }

  //LOG (INFO, "Preparing to open or create files.");
  for (size_t i=0; i < numOfFiles; i++)
    {
      fileStreams.push_back(new fstream());
      fileMutexes.push_back(movable_mutex());
      //fstream fileStream (saveFiles[i], ios::in | ios::out | ios::app | ios::binary | ios::ate);

      if (!openOrCreateFile(saveFiles[i], torrentFiles[i].getLength(), fileStreams[i]))
	{
	  closeOpenFiles();
	  break;
	}
    }
}

void FileHandler::closeOpenFiles()
{
  size_t numOfOpenFiles = fileStreams.size();
  for (size_t i=0; i<numOfOpenFiles; i++)
    {
      if (fileStreams[i]->is_open())
	fileStreams[i]->close();
    }
  fileStreams.clear();
  fileMutexes.clear();
  LOG (DEBUG, "FileHandler : Closed file handles.");
}

FileHandler::~FileHandler()
{
  size_t numOfOpenFiles = fileStreams.size();
  for (size_t i=0; i < numOfOpenFiles; i++)
    {
      if (fileStreams[i]->is_open())
	fileStreams[i]->close();
    }
  fileStreams.clear();
  fileMutexes.clear();
  LOG (DEBUG, "FileHandler : Closed file handles.");
}

bool FileHandler::openOrCreateFile(string filename, size_t filesize_expected, fstream *filestream)
{
  filestream->open (filename, ios::in | ios::out | ios::app | ios::binary | ios::ate); // ate => seek pointer at eof
	
  if (filestream->is_open())
    {
      LOG (INFO, "FileHandler : Opened file " + filename);
      streampos filesize_actual = filestream->tellg();	// this is where ios::ate proves useful
  		
      if (filesize_actual == 0) // allocate space for the file
	{
	  LOG (DEBUG, "FileHandler : Allocating space for file " + filename);
	  vector<char> zeroes(BLOCK_SIZE, 0);
	  for (size_t i=0; i < filesize_expected/zeroes.size(); i++)
	    {
	      if (!filestream->write(&zeroes[0], zeroes.size()))
		{
		  LOG(ERROR, "FileHandler : Error writing zeroes to file " + filename);
		  filestream->close();
		  return false;
		}
	    }
	  if (!filestream->write(&zeroes[0], filesize_expected % zeroes.size()))
	    {
	      LOG(ERROR, "FileHandler : Error writing zeroes to file " + filename);
	      filestream->close();
	      return false;
	    }
	}
      else if (filesize_actual != (streampos) filesize_expected) // if file exists but of improper size, abort
	{
	  LOG (ERROR, "FileHandler : File " + filename + " already exists and possibly improperly initialized.");
	  filestream->close();
	  return false;
	}
		
      // by now file already exists and is of correct size => close and reopen file in required modes
      filestream->close();
      filestream->open (filename, ios::in | ios::out | ios::binary);
      return filestream->is_open();
    }
  else
    {
      LOG (ERROR, "FileHandler : Failed to open file " + filename);
      return false;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool FileHandler::readPiece(size_t pieceId, string& piece, size_t &readLength)
{
  bool pieceComplete = false;
  size_t lengthRead = 0;
  piece = "";
  vector<TorrentFile> torrentFiles = torrent.getFiles();
  for (size_t i=0; i < fileStreams.size(); i++) // For each available, open file stream
    {
      //LOG (DEBUG, "FileHandler : Piece Length : " + to_string(torrent.getPieceLength()));
      size_t pieceStart = pieceId * torrent.getPieceLength();
      //LOG (DEBUG, "FileHandler : Piece#" + to_string(pieceId) + " start-offset : " + to_string(pieceStart));
      size_t pieceEnd = readLength > 0 && readLength < torrent.getPieceLength() ? pieceStart + readLength : pieceStart + torrent.getPieceLength();
      //LOG (DEBUG, "FileHandler : Piece#" + to_string(pieceId) + " max-end-offset : " + to_string(pieceEnd));
      size_t fileStart = i == 0 ? 0 : torrentFiles[i-1].getLength();
      //LOG (DEBUG, "FileHandler : File#" + to_string(i) + " start-offset : " + to_string(fileStart));
      size_t fileEnd = fileStart + torrentFiles[i].getLength();
      //LOG (DEBUG, "FileHandler : File#" + to_string(i) + " end-offset : " + to_string(fileEnd));

      if (fileEnd > pieceStart) 		// piece spans this file (and possibly following file(s))
	{
	  size_t readStart = 0;
	  if (fileStart <= pieceStart) 	// if piece starts in this file
	    readStart = pieceStart - fileStart;
	  else
	    readStart = 0;
	  //LOG (DEBUG, "FileHandler : Piece#" + to_string(pieceId) + " read-from-offset : " + to_string(readStart));
	  fileMutexes[i].lock();
	  fileStreams[i]->seekg(readStart, ios::beg); // start at readStart

	  size_t readEnd = fileEnd;
	  if (fileEnd >= pieceEnd) 	// piece ends in this file
	    {
	      readEnd = pieceEnd;	// read only upto what is requested
	      pieceComplete = true; 	// no more files needed to fill piece data
	    }
	  else
	    {
	      readEnd = fileEnd; 	// cannot read more than fileEnd offset
	      pieceComplete = (i+1 == fileStreams.size()); 	// if last piece then it isComplete; otherwise it's not
	    }
	  //LOG (DEBUG, "FileHandler : Piece#" + to_string(pieceId) + " read-to-offset : " + to_string(readEnd));

	  LOG (DEBUG, "FileHandler : Reading " + to_string(readEnd - readStart) + " bytes [" + to_string(readStart) + " - " + to_string(readEnd) + "] from file#" + to_string(i) + " [" + to_string(fileStart) + " - " + to_string(fileEnd) + "] into piece#" + to_string(pieceId));
	  char *buffer = new char[readEnd - readStart];
	  fileStreams[i]->read(buffer, readEnd - readStart); 	// read computed range into buffer
	  fileMutexes[i].unlock();
	  string bufferString(buffer, readEnd - readStart); 	// required step for binary file
	  piece += bufferString; 					// append to piece
	  lengthRead += readEnd - readStart;
	  delete[] buffer;
			
	  if (pieceComplete) 					// if piece populated, break
	    break;
	}
    }
  readLength = lengthRead;
  return pieceComplete;
}

size_t FileHandler::writePiece(size_t pieceId, string piece, size_t writeLength, size_t startOffset)
{
  //LOG (DEBUG, "Data Length : " + to_string(piece.size()));
  bool pieceComplete = false;
  size_t lengthWritten = 0;
  vector<TorrentFile> torrentFiles = torrent.getFiles();
  for (size_t i=0; i < fileStreams.size(); i++) // For each available, open file stream
    {
      //LOG (DEBUG, "FileHandler : Piece Length : " + to_string(torrent.getPieceLength()));
      size_t pieceStart = pieceId * torrent.getPieceLength();
      //LOG (DEBUG, "FileHandler : Piece#" + to_string(pieceId) + " start-offset : " + to_string(pieceStart));
      size_t pieceEnd = writeLength > 0 && writeLength < torrent.getPieceLength() ? pieceStart + writeLength : pieceStart + torrent.getPieceLength();
      //LOG (DEBUG, "FileHandler : Piece#" + to_string(pieceId) + " max-end-offset : " + to_string(pieceEnd));
      size_t fileStart = i == 0 ? 0 : torrentFiles[i-1].getLength();
      //LOG (DEBUG, "FileHandler : File#" + to_string(i) + " start-offset : " + to_string(fileStart));
      size_t fileEnd = fileStart + torrentFiles[i].getLength();
      //LOG (DEBUG, "FileHandler : File#" + to_string(i) + " end-offset : " + to_string(fileEnd));

      if (fileEnd > pieceStart) 		// piece spans this file (and possibly following file(s))
	{
	  size_t writeStart = 0;
	  if (fileStart <= pieceStart) 	// if piece starts in this file
	    writeStart = pieceStart - fileStart;
	  else
	    writeStart = 0;
	  //LOG (DEBUG, "FileHandler : Piece#" + to_string(pieceId) + " read-from-offset : " + to_string(readStart));
	  fileMutexes[i].lock();
	  fileStreams[i]->seekp(writeStart, ios::beg); // start at writeStart

	  size_t writeEnd = fileEnd;
	  if (fileEnd >= pieceEnd) 	// piece ends in this file
	    {
	      writeEnd = pieceEnd;	// write only upto pieceEnd
	      pieceComplete = true; 	// no more files needed to write piece data
	    }
	  else
	    {
	      writeEnd = fileEnd; 	// cannot write more than fileEnd offset
	      pieceComplete = (i+1 == fileStreams.size()); 	// if last piece then it isComplete; otherwise it's not
	    }
	  //LOG (DEBUG, "FileHandler : Piece#" + to_string(pieceId) + " read-to-offset : " + to_string(readEnd));
	  if (piece.size() >= lengthWritten + writeEnd - writeStart)
	    {
	      LOG (DEBUG, "FileHandler : Writing " + to_string(writeEnd - writeStart) + " bytes [" + to_string(writeStart) + " - " + to_string(writeEnd) + "] to file#" + to_string(i) + " [" + to_string(fileStart) + " - " + to_string(fileEnd) + "] from piece#" + to_string(pieceId));	
	      string writeBuffer = piece.substr(lengthWritten, writeEnd - writeStart);
	      fileStreams[i]->write(writeBuffer.c_str(), writeBuffer.size()); 	// write the buffer into file
	      lengthWritten += writeBuffer.size();
	      fileStreams[i]->flush();
	      fileMutexes[i].unlock();

	      if (pieceComplete) 	// if available piece data written completely, break
		break;
	    }
	  else
	    {
	      LOG (ERROR, "FileManager : Insufficient piece data (" + to_string(piece.size()) + ") for writing to file#" + to_string(i));
	      break;
	    }
	}
    }
  return lengthWritten;
}

bool FileHandler::isValidPiece(size_t pieceId)
{
  string piece;
  return readIfValidPiece(pieceId, piece);
  /*size_t pieceLength = torrent.getPieceLength();
    unsigned char pieceHash[20];
	
    if (readPiece(pieceId, piece, pieceLength))
    {
    SHA1((unsigned char *)piece.c_str(), pieceLength, pieceHash);
    //LOG (DEBUG, "FileHandler : Piece#" + to_string(pieceId) + " hash = " + string(pieceHash));
    return checkHashes(&pieceHash[0], (unsigned char *)torrent.pieceHashAt(pieceId).c_str(), 20);
    }
    return false;*/
}

bool FileHandler::readIfValidPiece(size_t pieceId, string& piece)
{	
  size_t pieceLength = torrent.getPieceLength();
  unsigned char pieceHash[20];
	
  if (readPiece(pieceId, piece, pieceLength))
    {
      SHA1((unsigned char *)piece.c_str(), pieceLength, pieceHash);
      //LOG (DEBUG, "FileHandler : Piece#" + to_string(pieceId) + " hash = " + string(pieceHash));
      return checkHashes(&pieceHash[0], (unsigned char *)torrent.pieceHashAt(pieceId).c_str(), 20);
    }
  return false;
}

bool FileHandler::checkHashes(unsigned char *hash1, unsigned char *hash2, size_t length)
{
  size_t i=0;
  for(; i < length && hash1[i] == hash2[i]; i++);
  if (i == length)
    LOG (DEBUG, "FileHandler : hash-check = passed");
  else
    LOG (DEBUG, "FileHandler : hash-check = failed");
  return i == length;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
