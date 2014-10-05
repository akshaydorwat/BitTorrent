/*
 * TorrentPiece_t.cpp
 *
 *  Created on: Oct 04, 2014
 *      Author: rkhapare
 */

#include "TorrentPiece_t.h"

//#include <iostream>
#include <string>
#include <vector>
//#include <ctime>
//#include <exception>
//#include <stdexcept>
//#include <cassert>
//#include <stdlib.h>
using namespace std;

TorrentPiece_t::TorrentPiece_t(size_t length, string pieceHash)
{
	setLength(length);
	setHash(pieceHash);
}

//////////////////////////////////////////////////////////////////////////////////////
size_t TorrentPiece_t::getLength()
{
	return length;
}

void TorrentPiece_t::setLength(size_t len)
{
	length = len;
}

//////////////////////////////////////////////////////////////////////////////////////
/*string TorrentPiece_t::getData()
{
	return data;
}

void TorrentPiece_t::setData(string str)
{
	data = str;
}
*/
//////////////////////////////////////////////////////////////////////////////////////
/*bool TorrentPiece_t::isComplete()
{
	return length == data.size();
}
*/
//////////////////////////////////////////////////////////////////////////////////////
void TorrentPiece_t::setHash(string pieceHash)
{
	hash = pieceHash;
}

string TorrentPiece_t::getHash()
{
	return hash;
}

//////////////////////////////////////////////////////////////////////////////////////
/*bool TorrentPiece_t::isValid()
{
	return isComplete() && data.size() > hash.size(); // edit : hash comparison
}
*/
///////////////////////////////////////////////////////////////////////////////////////////////////
