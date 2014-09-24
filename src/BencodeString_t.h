/*
 * BencodeString_t.h
 *
 *  Created on: Sep 21, 2014
 *      Author: rkhapare
 */

#ifndef BENCODESTRING_T_H_
#define BENCODESTRING_T_H_

#include "Bencode_t.h"
#include <string>

using namespace std;

class BencodeString_t : public Bencode_t
{
	private:
			string data;

	public:
			BencodeString_t(string);

			string get();
			void set(string);
			friend ostream& operator<<(ostream&, const BencodeString_t&);

			virtual BencodeString_t* clone() const;
			string display();
			string encode(); // implement the Bencode_t interface
			static BencodeString_t decode(string, size_t*);
};

#endif /* BENCODESTRING_T_H_ */
