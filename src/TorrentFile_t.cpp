/*
 * TorrentFile_t.cpp
 *
 *  Created on: Sep 22, 2014
 *      Author: rkhapare
 */

#include "TorrentFile_t.h"

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
using namespace std;

size_t TorrentFile_t::getLength()
{
	return length;
}

void TorrentFile_t::setLength(unsigned int len)
{
	length = len;
}

// info
// d files l d length 19920192
//             path
//                   l  Total Video Converter HD v3.71 + Serials [ChattChitto RG].exe
void TorrentFile_t::setLength(BencodeDictionary_t *infoOrFilesDictionary)
{
//	BencodeDictionary_t* torrentDictionary =
//			dynamic_cast<BencodeDictionary_t*>(bencodeTorrent_t);
//	BencodeDictionary_t *infoDictionary =
//			dynamic_cast<BencodeDictionary_t*>(torrentDictionary->at("info"));
//	BencodeList_t *filesList = dynamic_cast<BencodeList_t*>(infoDictionary->at(
//			"files"));
//
//	/* NOTE:files is a list of dictionaries. eg. in bencoding: ld6:lengthi1024e4path:l8:filenameeee
//	 * Currently extracting only the first dictionary from amongst the list of dictionaries. */
//	BencodeDictionary_t *filesDictionary =
//			dynamic_cast<BencodeDictionary_t*>(filesList->at(0));
	BencodeInteger_t *lengthInteger =
			dynamic_cast<BencodeInteger_t*>(infoOrFilesDictionary->at("length"));
	if (lengthInteger)
	{
		length = lengthInteger->get();
	}
	else
	{
		throw invalid_argument("Failed to find \"length\" object !!!");
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
vector<string> TorrentFile_t::getPaths()
{
	return paths;
}

void TorrentFile_t::setPaths(BencodeDictionary_t *infoOrFilesDictionary)
{
//	BencodeDictionary_t* torrentDictionary =
//			dynamic_cast<BencodeDictionary_t*>(bencodeTorrent_t);
//	BencodeDictionary_t *infoDictionary =
//			dynamic_cast<BencodeDictionary_t*>(torrentDictionary->at("info"));
//	BencodeList_t *filesList = dynamic_cast<BencodeList_t*>(infoDictionary->at(
//			"files"));
//
//	/* NOTE:files is a list of dictionaries. eg. in bencoding: ld6:lengthi1024e4path:l8:filenameeee
//	 * Currently extracting only the first dictionary from amongst the list of dictionaries. */
//	BencodeDictionary_t *filesDictionary =
//			dynamic_cast<BencodeDictionary_t*>(filesList->at(0));
	BencodeList_t *pathList =
			dynamic_cast<BencodeList_t*>(infoOrFilesDictionary->at("path"));
	if (pathList)
	{
		for (size_t i = 0; i < pathList->size(); i++)
		{
			BencodeString_t *bencodeString_t =
					dynamic_cast<BencodeString_t*>(pathList->at(i));
			if (bencodeString_t)
			{
			paths.push_back(bencodeString_t->get());
			}
			else
			{
				throw out_of_range("Failed to parse \"path\" object !!!");
			}
		}
	}
	else
	{
		throw invalid_argument("Failed to find \"path\" object !!!");
	}
}

void TorrentFile_t::addPath(string path)
{
	paths.push_back(path);
}

string TorrentFile_t::pathAt(unsigned int i)
{
	return paths[i];
}

///////////////////////////////////////////////////////////////////////////////////////////////////
