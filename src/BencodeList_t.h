/*
 * BencodeList_t.h
 *
 *  Created on: Sep 21, 2014
 *      Author: rkhapare
 */

#ifndef BENCODELIST_T_H_
#define BENCODELIST_T_H_

#include "Bencode_t.h"

#include <vector>
#include <string>

using namespace std;

class BencodeList_t: public Bencode_t
{
	private:
		vector<Bencode_t*> data;

	public:
		size_t size();
		Bencode_t* at(int);
		vector<Bencode_t*> get();
		void add(Bencode_t*);
		friend ostream& operator<<(ostream&, const BencodeList_t&);

		virtual BencodeList_t* clone() const;
		string display();
		string encode();
		static BencodeList_t decode(string, size_t*);
};

#endif /* BENCODELIST_T_H_ */
