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
//#include <mutex>
//#include <ctime>
//#include <exception>
//#include <stdexcept>
//#include <cassert>
//#include <stdlib.h>
using namespace std;

//mutex availableMtx, processingMtx;

FileHandler::FileHandler(Torrent& torrent, vector<string> saveFiles)
:torrent(torrent)
,saveFiles(saveFiles)
{
	size_t numOfFiles = 1; // torrent.getFiles().size();
	vector<TorrentFile> torrentFiles = torrent.getFiles();
	if (saveFiles.size() < numOfFiles)
	{	
		for(size_t i=saveFiles.size(); i < numOfFiles; i++)
			saveFiles.push_back(torrentFiles[i].pathAt(0));
	}

	for (size_t i=0; i < numOfFiles; i++)
		openOrCreateFile(saveFiles[i], torrentFiles[i].getLength(), fileStreams[i]);
}

FileHandler::~FileHandler()
{
	size_t numOfFiles = saveFiles.size();
	for (size_t i=0; i < numOfFiles; i++)
		fileStreams[i].close();
}

bool FileHandler::openOrCreateFile(string filename, size_t filesize_expected, fstream& filestream)
{
	filestream.open (filename, ios::in | ios::out | ios::app | ios::binary | ios::ate);
	if (filestream.is_open())
	{
		streampos filesize_actual = filestream.tellg();
  		//filestream.seekg (0, ios::end);
  		//end = filestream.tellg();
		if (filesize_actual == 0) // allocate space for the file
		{
			vector<char> zeroes(BLOCK_SIZE, 0);
			for (size_t i=0; i < filesize_expected/zeroes.size(); i++)
			{
				if (!filestream.write(&zeroes[0], zeroes.size()))
				{
					LOG(ERROR, "Error writing zeroes to file " + filename);
					return false;
				}
			}
			if (!filestream.write(&zeroes[0], filesize_expected % zeroes.size()))
			{
				LOG(ERROR, "Error writing zeroes to file " + filename);
				return false;
			}
			return true;
		}
		else if (filesize_actual != (streampos) filesize_expected) // if file exists but of improper size, abort
		{
			LOG (ERROR, "File " + filename + " already exists and possibly improperly initialized.");
			return false;
		}
	}
	return false;
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
		size_t pieceStart = pieceId * torrent.getPieceLength();
		size_t pieceMaxLength = readLength > 0 && readLength < torrent.getPieceLength() ? pieceStart + readLength : pieceStart + torrent.getPieceLength();
		size_t fileStart = i == 0 ? 0 : torrentFiles[i-1].getLength();
		size_t fileLength = torrentFiles[i].getLength();

		if (fileLength > pieceStart) // piece spans this file (and possibly following file(s))
		{
			size_t readStart = 0;
			if (fileStart <= pieceStart) // if piece starts in this file
				readStart = pieceStart - fileStart;
			else
				readStart = 0;
			fileStreams[i].seekg(readStart, ios::beg); // start at readStart

			size_t readEnd = fileLength;
			if ((fileStart + fileLength) >= pieceMaxLength) // piece ends in this file
			{
				readEnd = (fileStart + fileLength) - pieceMaxLength;
				pieceComplete = true; // no more files needed to fill piece data
			}
			else
			{
				readEnd = fileLength;
				pieceComplete = (i+1 == fileStreams.size()); // if last piece then it isComplete; otherwise it's not
			}

			char *buffer = new char[readEnd - readStart];
			fileStreams[i].read(buffer, readEnd - readStart); // read computed range into buffer
			string bufferString(buffer, readEnd - readStart); // required step for binary file
			piece += bufferString; // append to piece
			lengthRead += readEnd - readStart;
			delete[] buffer;
			
			if (pieceComplete) // if piece populated, break
				break;
		}
	}
	readLength = lengthRead;
	return pieceComplete;
}

bool FileHandler::isValidPiece(size_t pieceId)
{
	string piece;
	size_t pieceLength = torrent.getPieceLength();
	unsigned char pieceHash[20];
	
	if (readPiece(pieceId, piece, pieceLength))
	{
		SHA1((unsigned char *)piece.c_str(), pieceLength, pieceHash);
		return checkHashes(&pieceHash[0], (unsigned char *)torrent.pieceHashAt(pieceId).c_str(), 20);
	}
	return false;
}

bool FileHandler::checkHashes(unsigned char *hash1, unsigned char *hash2, size_t length)
{
	size_t i=0;
	for(; i < length && hash1[i] == hash2[i]; i++);
	return i == length;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
