/*
 * BencodeList_t.cpp
 *
 *  Created on: Sep 21, 2014
 *      Author: rkhapare
 */

#include "BencodeList_t.h"
#include "Bencode_t.h"
#include "BencodeString_t.h"
#include "BencodeInteger_t.h"
#include "BencodeDecoder.h"
#include "Logger.hpp"

#include <iostream>
#include <string>
#include <sstream>
#include <stdlib.h>
#include <cassert>

using namespace std;

BencodeList_t* BencodeList_t::clone() const
{
	return new BencodeList_t(*this);
}

//BencodeList_t& operator=(BencodeList_t const& bencodeList_t)
//{
//	if (this != &bencodeList_t)
//	{ // Check for self-assignment
//		for (size_t i=0; i < bencodeList_t.size(); i++)
//		{
//			Bencode_t* p2 = bencodeList_t.at(i)->clone(); // Create the new one FIRST...
//			delete p_; // ...THEN delete the old one
//			p_ = p2;
//		}
//	}
//	return *this;
//}

size_t BencodeList_t::size()
{
	return data.size();
}

Bencode_t* BencodeList_t::at(int idx)
{
	return data.at(idx);
}

vector<Bencode_t*> BencodeList_t::get()
{
	return data;
}

void BencodeList_t::add(Bencode_t *elm)
{
//	size_t s = data.size();
//	data.resize(s + 1);
	data.push_back(elm);
//	cout << "List size = " << data.size() << endl;
}

ostream& operator<<(ostream& os, const BencodeList_t& bencodeList_t)
{
	os << "{ ";
	for (size_t i = 0; i < bencodeList_t.data.size(); ++i)
	{
		os << "(" << i << ") ";
		os << bencodeList_t.data[i]->display();
		if (i < bencodeList_t.data.size() - 1)
			os << ", ";
	}
	os << " }";

	return os;
}

string BencodeList_t::display()
{
	string temp = "{ ";
	for (size_t i = 0; i < size(); ++i)
	{
		stringstream ss;
		ss << "(" << i << ") ";
		temp += ss.str();
		temp += ((at(i))->display());
		if (i < size() - 1)
			temp += ", ";
	}
	temp += " }";

	return temp;
}

// implement the interface of Bencode_t
string BencodeList_t::encode()
{
	string bencodedString = "l";

	for (size_t i = 0; i < size(); ++i)
	{
		bencodedString += ((at(i))->encode());
	}

	bencodedString += "e";
	return bencodedString;
}

BencodeList_t BencodeList_t::decode(string bencodeList, size_t *startIdx)
{
	assert (*startIdx >= 0 && *startIdx < bencodeList.size());
	
		string operableBencodeList = bencodeList.substr(*startIdx);

//		cout << "----------------------------------------------------" << endl;
//		for (size_t i = 0; i < *startIdx; i++)
//		{
//			cout << ' ';
//		}
//		cout << 'v' << endl;
//		cout << bencodeList << endl;

		size_t lIdx = operableBencodeList.find('l', 0);
		size_t eIdx = operableBencodeList.find('e', 0);

		assert (lIdx != string::npos && eIdx != string::npos && eIdx > lIdx + 3);
		
			*startIdx += lIdx + 1;
			BencodeList_t bencodeList_t;

			while (bencodeList[*startIdx] != 'e')
			{
				Bencode_t *bencode_t = BencodeDecoder::decode(bencodeList,
						startIdx);
				bencodeList_t.add(bencode_t);
			}

			*startIdx += 1;

//			for (size_t i = 0; i < *startIdx; i++)
//			{
//				cout << ' ';
//			}
//			cout << 'v' << endl;
//			cout << bencodeList << endl;
//
//			cout << "List : " << bencodeList_t << endl;
			LOG (DEBUG, "List : " + bencodeList_t.display());

			return bencodeList_t;	
}

