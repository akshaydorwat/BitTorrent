/*
 * BencodeDictionary_t.cpp
 *
 *  Created on: Sep 21, 2014
 *      Author: rkhapare
 */

#include "BencodeDictionary_t.h"
#include "Bencode_t.h"
#include "BencodeString_t.h"
#include "BencodeDecoder.h"
#include "Logger.hpp"

#include <iostream>
#include <string>
#include <map>
#include <cassert>

using namespace std;

BencodeDictionary_t* BencodeDictionary_t::clone() const
{
  return new BencodeDictionary_t(*this);
}

Bencode_t* BencodeDictionary_t::at(string key)
{
  //	assert (data.find(key) != data.end());
  //	{
  return data[key];
  //	}
}

map<string, Bencode_t*> BencodeDictionary_t::get()
{
  return data;
}

void BencodeDictionary_t::put(string key, Bencode_t* value)
{
  data[key] = value;
}

ostream& operator<<(ostream& os, const BencodeDictionary_t& bencodeDictionary_t)
{
  os << "\n{";
  for (map<string, Bencode_t*>::const_iterator itr = bencodeDictionary_t.data.begin(); itr != bencodeDictionary_t.data.end(); ++itr)
    {
      os << "\n\t[ ";
      os << itr->first;
      os << " : ";
      os << ((itr->second)->display());
      os << " ]";
    }
  os << "\n}\n";
  return os;
}

string BencodeDictionary_t::display()
{
  string temp = "\n{";

  for (map<string, Bencode_t*>::iterator itr = data.begin();
       itr != data.end(); ++itr)
    {
      temp += "\n\t[ ";
      temp += itr->first;
      temp += " : ";
      temp += ((itr->second)->display());
      temp += " ]";
    }

  temp += "\n}\n";
  return temp;
}

string BencodeDictionary_t::encode()
{
  string bencodedString = "d";

  for (map<string, Bencode_t*>::iterator itr = data.begin();
       itr != data.end(); ++itr)
    {
      BencodeString_t key(itr->first);
      bencodedString += (key.encode());
      //bencodedString += ":";
      bencodedString += ((itr->second)->encode());
    }

  bencodedString += "e";
  return bencodedString;
}

BencodeDictionary_t BencodeDictionary_t::decode(string bencodeDictionary,
						size_t* startIdx)
{
  assert (*startIdx >= 0 && *startIdx < bencodeDictionary.size());
	
  string operableBencodeDictionary = bencodeDictionary.substr(*startIdx);

  //		cout << "----------------------------------------------------" << endl;
  //		for (size_t i = 0; i < *startIdx; i++)
  //		{
  //			cout << ' ';
  //		}
  //		cout << 'v' << endl;
  //		cout << bencodeDictionary << endl;

  size_t dIdx = operableBencodeDictionary.find('d', 0);
  size_t eIdx = operableBencodeDictionary.find('e', 0);

  assert (dIdx != string::npos && eIdx != string::npos && eIdx > dIdx + 3);
		
  *startIdx += dIdx + 1;
  BencodeDictionary_t bencodeDictionary_t;

  while (bencodeDictionary[*startIdx] != 'e')
    {
      BencodeString_t bencodeString_t = BencodeString_t::decode(
								bencodeDictionary, startIdx);
      bencodeDictionary_t.put(bencodeString_t.get(),
			      BencodeDecoder::decode(bencodeDictionary, startIdx));
    }

  *startIdx += 1;

  //			for (size_t i = 0; i < *startIdx; i++)
  //			{
  //				cout << ' ';
  //			}
  //			cout << 'v' << endl;
  //			cout << bencodeDictionary << endl;
  //
  //			cout << "Dictionary :" << bencodeDictionary_t << endl;
  //LOG (DEBUG, "Dictionary : \n" + bencodeDictionary_t.display());

  return bencodeDictionary_t;
}
