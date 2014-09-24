/*
 * BencodeString_t.cpp
 *
 *  Created on: Sep 21, 2014
 *      Author: rkhapare
 */

#include "BencodeString_t.h"

#include <iostream>
#include <string>
//#include <cstring>
#include <stdlib.h>

using namespace std;

BencodeString_t::BencodeString_t(string data)
		: data(data)
{
}

BencodeString_t* BencodeString_t::clone() const
{
  return new BencodeString_t(*this);
}

string BencodeString_t::get()
{
	return data;
}

void BencodeString_t::set(string str)
{
	data = str;
}

ostream& operator<<(ostream& os, const BencodeString_t& bencodeString_t)
{
    os << bencodeString_t.data;
    return os;
}

string BencodeString_t::display()
{
	return data;
}

//bool BencodeString_t::operator<(BencodeString_t b)
//{
//   return strcmp(get().c_str(), b.get().c_str()) < 0;
//}

// implement the interface of Bencode_t
string BencodeString_t::encode()
{
	return data.size() + ":" + data;
}

BencodeString_t BencodeString_t::decode(string bencodeString, size_t *startIdx)
{
	if (*startIdx >= 0 && *startIdx < bencodeString.size())
	{
		string operableBencodeString = bencodeString.substr(*startIdx);

//		cout << "----------------------------------------------------" << endl;
//		for (size_t i = 0; i < *startIdx; i++)
//		{
//			cout << ' ';
//		}
//		cout << 'v' << endl;
//		cout << bencodeString << endl;

		size_t colonIdx = operableBencodeString.find(':', 0);
		if (colonIdx != string::npos)
		{
			int dataLen = atoi(operableBencodeString.substr(0, colonIdx).c_str());
			string data = operableBencodeString.substr(colonIdx + 1, dataLen);
			*startIdx += colonIdx + 1 + dataLen;

			BencodeString_t bencodeString_t (data);

//			for (size_t i = 0; i < *startIdx; i++)
//			{
//				cout << ' ';
//			}
//			cout << 'v' << endl;
//			cout << bencodeString << endl;
//
//			cout << "String : " << bencodeString_t << endl;

			return bencodeString_t;
		}
	}
}
