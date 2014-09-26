/*
 * Bencode_t.h
 *
 *  Created on: Sep 21, 2014
 *      Author: rkhapare
 */

#ifndef BENCODE_T_H_
#define BENCODE_T_H_

#include <string>
using namespace std;

class Bencode_t
{
	public:
		virtual ~Bencode_t();

		// make interface with a pure virtual method
		virtual string encode() = 0;
		virtual string display() = 0;
		virtual Bencode_t* clone() const = 0;   // The Virtual (Copy) Constructor
};

#endif /* BENCODE_T_H_ */
