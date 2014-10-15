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
#include <mutex>
using namespace std;

///////////////////////////////////////////////////////////////////////////////////////////////////
class FileHandler
{
	public:
		struct movable_mutex : std::mutex
		{
			movable_mutex() = default;
			movable_mutex(movable_mutex const&) noexcept : std::mutex() {}
			bool operator==(movable_mutex const&other) noexcept { return this==&other; }
		};

	private:
		Torrent& torrent;
		vector<string>& saveFiles;
		vector<fstream*> fileStreams;
		vector<movable_mutex> fileMutexes;

	public:
		FileHandler(Torrent&, vector<string>&);
		~FileHandler();
		void closeOpenFiles();

		bool openOrCreateFile(string, size_t, fstream*);
		bool readPiece(size_t, string &, size_t &);
		size_t writePiece(size_t, string, size_t, size_t);
		bool isValidPiece(size_t);
		bool readIfValidPiece(size_t, string &);
		bool checkHashes(unsigned char *, unsigned char *, size_t);
};

#endif /* FILEHANDLER_HPP_ */
///////////////////////////////////////////////////////////////////////////////////////////////////
