/*
 * BencodeInteger_t.cpp
 *
 *  Created on: Sep 21, 2014
 *      Author: rkhapare
 */

#include "BencodeInteger_t.h"

#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

BencodeInteger_t::BencodeInteger_t(int data)
		: data(data)
{
}

BencodeInteger_t* BencodeInteger_t::clone() const
{
  return new BencodeInteger_t(*this);
}

int BencodeInteger_t::get()
{
	return data;
}

void BencodeInteger_t::set(int temp)
{
	data = temp;
}

ostream& operator<<(ostream& os, const BencodeInteger_t& bencodeInteger_t)
{
    os << bencodeInteger_t.data;
    return os;
}

string BencodeInteger_t::display()
{
	char buffer[33];
	sprintf(buffer, "%d", data);
	string temp(buffer);
	return temp;
}

string BencodeInteger_t::encode()
{
	string temp = "i";
	temp += data;
	temp += "e";
	return temp;
}

BencodeInteger_t BencodeInteger_t::decode(string bencodeInteger,
		size_t *startIdx)
{
	if (*startIdx >= 0 && *startIdx < bencodeInteger.size())
	{
		string operableBencodeInteger = bencodeInteger.substr(*startIdx);

//		cout << "----------------------------------------------------" << endl;
//		for (size_t i = 0; i < *startIdx; i++)
//		{
//			cout << ' ';
//		}
//		cout << 'v' << endl;
//		cout << bencodeInteger << endl;

		size_t iIdx = operableBencodeInteger.find('i', 0);
		size_t eIdx = operableBencodeInteger.find('e', 0);
		if (iIdx != string::npos && eIdx != string::npos && eIdx > iIdx + 1)
		{
			int data = atoi(operableBencodeInteger.substr(iIdx + 1, eIdx - 1).c_str());
			*startIdx += eIdx + 1;

			BencodeInteger_t bencodeInteger_t(data);

//			for (size_t i = 0; i < *startIdx; i++)
//			{
//				cout << ' ';
//			}
//			cout << 'v' << endl;
//			cout << bencodeInteger << endl;
//
//			cout << "Integer : " << bencodeInteger_t << endl;

			return bencodeInteger_t;
		}
	}
}
