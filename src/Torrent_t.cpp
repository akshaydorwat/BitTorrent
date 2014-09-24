/*
 * Torrent_t.cpp
 *
 *  Created on: Sep 22, 2014
 *      Author: rkhapare
 */

#include "Torrent_t.h"
#include "TorrentFile_t.h"
#include "BencodeDecoder.h"
#include "Bencode_t.h"
#include "BencodeDictionary_t.h"
#include "BencodeList_t.h"
#include "BencodeString_t.h"
#include "BencodeInteger_t.h"

#include <iostream>
#include <string>
#include <vector>
#include <ctime>
#include <exception>
#include <stdexcept>
#include <cassert>
#include <stdlib.h>
#include <fstream>
using namespace std;

Torrent_t Torrent_t::decode(string torrentString)
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

	cout << bencode_t->display() << endl;

	Torrent_t newTorrent;
	try
	{
		cout << "TORRENT PROPERTY\t:\t\tVALUE(S)" << endl << endl;

		newTorrent.setName(bencode_t);
		cout << "Name\t\t\t:\t\t" << newTorrent.getName() << endl << endl;

		try
		{
			newTorrent.setCreator(bencode_t);
			cout << "Creator\t\t\t:\t\t" << newTorrent.getCreator() << endl
					<< endl;
		}
		catch (const exception& e)
		{
		}

		try
		{
			newTorrent.setCreatedOn(bencode_t);
			cout << "Creation Date\t\t:\t\t" << newTorrent.showCreatedOn()
					<< endl;
		}
		catch (const exception& e)
		{
		}

		try
		{
			newTorrent.setAnnounce(bencode_t);
			cout << "Announce\t\t:\t\t" << newTorrent.getAnnounce() << endl
					<< endl;
		}
		catch (const exception& e)
		{
		}

		try
		{
			newTorrent.setAnnounceList(bencode_t);
			vector<string> announceList = newTorrent.getAnnounceList();
			cout << "Announce List (" << announceList.size() << ")\t:\t\t";
			for (size_t i = 0; i < announceList.size(); i++)
			{
				cout << announceList.at(i);
				if (i < announceList.size() - 1)
					cout << ",\n\t\t\t\t\t";
			}
			cout << endl << endl;
		}
		catch (const exception& e)
		{
		}

		try
		{
			newTorrent.setComment(bencode_t);
			cout << "Comment\t\t\t:\t\t" << newTorrent.getComment() << endl
					<< endl;
		}
		catch (const exception& e)
		{
		}

		try
		{
			newTorrent.setEncoding(bencode_t);
			cout << "Encoding\t\t:\t\t" << newTorrent.getEncoding() << endl
					<< endl;
		}
		catch (const exception& e)
		{
		}

		newTorrent.setPieceLength(bencode_t);
		cout << "Piece Length\t\t:\t\t" << newTorrent.getPieceLength() << endl
				<< endl;

		BencodeDictionary_t* torrentDictionary =
				dynamic_cast<BencodeDictionary_t*>(bencode_t);
		BencodeDictionary_t *infoDictionary =
				dynamic_cast<BencodeDictionary_t*>(torrentDictionary->at("info"));
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
		vector<TorrentFile_t> files = newTorrent.getFiles();
		cout << "Files (" << files.size() << ")\t\t:\t\t";
		for (size_t i = 0; i < files.size(); i++)
		{
			cout << files.at(i).pathAt(0) << "\t(" << files.at(i).getLength()
					<< ")";
			if (i < files.size() - 1)
				cout << ",\n\t\t\t\t\t";
		}
		cout << endl << endl;

		newTorrent.setPieceHashes(bencode_t);
		vector<string> pieceHashes = newTorrent.getPieceHashes();
		cout << "Piece Hashes (" << pieceHashes.size() << ")\t:\t\t";
		for (size_t i = 0; i < pieceHashes.size(); i++)
		{
			cout << pieceHashes.at(i);
			if (i < pieceHashes.size() - 1)
				cout << ",\n\t\t\t\t\t";
		}
		cout << endl << endl;
	}
	catch (const exception& e)
	{
		cout << "Failed to parse torrent !!! Invalid Format !!!\n" << e.what()
				<< endl;
	}

	return newTorrent;
}

/////////////////////////////////////////////////////////////////////////////////
string Torrent_t::getAnnounce()
{
	return announce;
}

void Torrent_t::setAnnounce(string str)
{
	announce = str;
}

void Torrent_t::setAnnounce(Bencode_t *bencodeTorrent_t)
{
	BencodeDictionary_t* torrentDictionary =
			dynamic_cast<BencodeDictionary_t*>(bencodeTorrent_t);
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
vector<string> Torrent_t::getAnnounceList()
{
	return announceList;
}

void Torrent_t::setAnnounceList(vector<string> list)
{
	announceList = list;
}

void Torrent_t::setAnnounceList(Bencode_t *bencodeTorrent_t)
{
	BencodeDictionary_t* torrentDictionary =
			dynamic_cast<BencodeDictionary_t*>(bencodeTorrent_t);
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

void Torrent_t::addToAnnounceList(string str)
{
	announceList.push_back(str);
}

/////////////////////////////////////////////////////////////////////////////////
string Torrent_t::getComment()
{
	return comment;
}

void Torrent_t::setComment(string str)
{
	comment = str;
}

void Torrent_t::setComment(Bencode_t *bencodeTorrent_t)
{
	BencodeDictionary_t* torrentDictionary =
			dynamic_cast<BencodeDictionary_t*>(bencodeTorrent_t);
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
string Torrent_t::getCreator()
{
	return creator;
}

void Torrent_t::setCreator(string str)
{
	creator = str;
}

void Torrent_t::setCreator(Bencode_t *bencodeTorrent_t)
{
	BencodeDictionary_t* torrentDictionary =
			dynamic_cast<BencodeDictionary_t*>(bencodeTorrent_t);
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
time_t Torrent_t::getCreatedOn()
{
	return createdOn;
}

string Torrent_t::showCreatedOn()
{
	return ctime(&createdOn);
}

void Torrent_t::setCreatedOn(Bencode_t *bencodeTorrent_t)
{
	BencodeDictionary_t* torrentDictionary =
			dynamic_cast<BencodeDictionary_t*>(bencodeTorrent_t);
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
string Torrent_t::getEncoding()
{
	return encoding;
}

void Torrent_t::setEncoding(string str)
{
	encoding = str;
}

void Torrent_t::setEncoding(Bencode_t *bencodeTorrent_t)
{
	BencodeDictionary_t* torrentDictionary =
			dynamic_cast<BencodeDictionary_t*>(bencodeTorrent_t);
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
string Torrent_t::getName()
{
	return name;
}

void Torrent_t::setName(string str)
{
	name = str;
}

void Torrent_t::setName(Bencode_t *bencodeTorrent_t)
{
	BencodeDictionary_t* torrentDictionary =
			dynamic_cast<BencodeDictionary_t*>(bencodeTorrent_t);
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
size_t Torrent_t::getPieceLength()
{
	return pieceLength;
}

void Torrent_t::setPieceLength(size_t size)
{
	pieceLength = size;
}

void Torrent_t::setPieceLength(Bencode_t *bencodeTorrent_t)
{
	BencodeDictionary_t* torrentDictionary =
			dynamic_cast<BencodeDictionary_t*>(bencodeTorrent_t);
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
vector<string> Torrent_t::getPieceHashes()
{
	return pieceHashes;
}

void Torrent_t::setPieceHashes(Bencode_t *bencodeTorrent_t)
{
	BencodeDictionary_t* torrentDictionary =
			dynamic_cast<BencodeDictionary_t*>(bencodeTorrent_t);
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
		}
	}
	else
	{
		throw invalid_argument("Failed to find \"pieces\" object !!!");
	}
}

void Torrent_t::addPieceHash(string str)
{
	pieceHashes.push_back(str);
}

string Torrent_t::pieceHashAt(int i)
{
	return pieceHashes[i];
}

/////////////////////////////////////////////////////////////////////////////////
vector<TorrentFile_t> Torrent_t::getFiles()
{
	return files;
}

TorrentFile_t Torrent_t::getFileAt(int i)
{
	assert(i >= 0 && i < files.size());
	return files[i];
}

void Torrent_t::addFile(TorrentFile_t file)
{
	files.push_back(file);
}

void Torrent_t::addFile(BencodeDictionary_t* filesDictionary)
{
	TorrentFile_t torrentFile_t;
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
