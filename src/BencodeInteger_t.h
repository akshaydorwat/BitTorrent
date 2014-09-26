/*
 * BencodeInteger_t.h
 *
 *  Created on: Sep 21, 2014
 *      Author: rkhapare
 */

#ifndef BENCODEINTEGER_T_H_
#define BENCODEINTEGER_T_H_

#include "Bencode_t.h"
#include <string>
using namespace std;

class BencodeInteger_t : public Bencode_t
{
	private:
		int data;

	public:
		BencodeInteger_t(int data);

		int get();
		void set(int);
		friend ostream& operator<<(ostream&, const BencodeInteger_t&);

		virtual BencodeInteger_t* clone() const;
		string display();
		string encode();
		static BencodeInteger_t decode(string, size_t *);
};

#endif /* BENCODEINTEGER_T_H_ */
