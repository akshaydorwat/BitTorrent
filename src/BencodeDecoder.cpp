/*
 * BencodeDecoder.cpp
 *
 *  Created on: Sep 21, 2014
 *      Author: rkhapare
 */

#include "BencodeDecoder.h"
#include "Bencode_t.h"
#include "BencodeString_t.h"
#include "BencodeInteger_t.h"
#include "BencodeList_t.h"
#include "BencodeDictionary_t.h"

#include <string>
#include <vector>
#include <iostream>
#include <stdexcept>

using namespace std;

vector<Bencode_t*> BencodeDecoder::decode(string bencode)
{
	size_t startIdx = 0;
	vector<Bencode_t*> bencodes(0);

	if (bencode.size() > 3)
	{
		while (startIdx < bencode.size())
		{
			bencodes.resize(bencodes.size() + 1);
			Bencode_t *temp = decode(bencode, &startIdx);
			bencodes.push_back(temp);
			cout << temp->display();
//			cout << "Start Index moved to " << startIdx << endl;
		}
	}

	return bencodes;
}

Bencode_t* BencodeDecoder::decode(string bencode, size_t *startIdx)
{
	Bencode_t *bencode_t;

	BencodeString_t bencodeString_t("");
	BencodeInteger_t bencodeInteger_t(0);
	BencodeList_t bencodeList_t;
	BencodeDictionary_t bencodeDictionary_t;

	if (bencode.size() > 3 && *startIdx >= 0)
//			&& *startIdx <= bencode.size())
	{
		switch (bencode[*startIdx])
		{
			// bencoded string eg. 5:hello => hello
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				bencodeString_t = BencodeString_t::decode(bencode, startIdx);
//				cout << bencodeString_t.display() << endl;
				bencode_t = bencodeString_t.clone();
				break;

			// bencoded integer eg. i100e => 100
			case 'i':
				bencodeInteger_t = BencodeInteger_t::decode(bencode, startIdx);
//				cout << bencodeInteger_t.display() << endl;
				bencode_t = bencodeInteger_t.clone();
				break;

			// bencoded list eg. l5:helloi100ee => { 'hello', 100 }
			case 'l':
				bencodeList_t = BencodeList_t::decode(bencode, startIdx);
//				cout << bencodeList_t.display() << endl;
				bencode_t = bencodeList_t.clone();
				break;

			// bencoded dictionary eg. d3:key5:valuee => key -> value
			case 'd':
				bencodeDictionary_t =BencodeDictionary_t::decode(bencode, startIdx);
//				cout << bencodeDictionary_t.display() << endl;
				bencode_t =  bencodeDictionary_t.clone();
				break;

			default:
				throw invalid_argument("Invalid Bencode type-identifier");
		}
		return bencode_t;
	}
	cout << "Insufficient Bencode length" << bencode.size() << ", " << *startIdx << endl;
	throw invalid_argument("Insufficient Bencode length");
}
