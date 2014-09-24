/*
 * BencodeDecoder.h
 *
 *  Created on: Sep 21, 2014
 *      Author: rkhapare
 */

#ifndef BENCODEDECODER_H_
#define BENCODEDECODER_H_

#include "Bencode_t.h"
#include <string>
#include <vector>

using namespace std;

class BencodeDecoder
{
	public:
		static vector<Bencode_t*> decode(string);
		static Bencode_t* decode(string, size_t *startIdx);
};

#endif /* BENCODEDECODER_H_ */
