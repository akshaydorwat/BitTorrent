/*
 * BencodeDictionary_t.h
 *
 *  Created on: Sep 21, 2014
 *      Author: rkhapare
 */

#ifndef BENCODEDICTIONARY_T_H_
#define BENCODEDICTIONARY_T_H_

#include "Bencode_t.h"
#include "BencodeString_t.h"

#include <string>
#include <map>

using namespace std;

class BencodeDictionary_t: public Bencode_t
{
	private:
		map<string, Bencode_t*> data;

	public:
		Bencode_t* at(string);
		map<string, Bencode_t*> get();
		void put(string, Bencode_t*);
		friend ostream& operator<<(ostream&, const BencodeDictionary_t&);

		virtual BencodeDictionary_t* clone() const;
		string display();
		string encode();
		static BencodeDictionary_t decode(string, size_t*);
};

#endif /* BENCODEDICTIONARY_T_H_ */
