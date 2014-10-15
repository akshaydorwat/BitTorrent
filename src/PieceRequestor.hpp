/*
 * PieceRequestor.hpp
 *
 *  Created on: Oct 11, 2014
 *      Author: rkhapare
 */

#ifndef PIECEREQUESTOR_HPP_
#define PIECEREQUESTOR_HPP_

#include "Piece.hpp"

#include <string>
#include <vector>
#include <mutex>
using namespace std;

///////////////////////////////////////////////////////////////////////////////////////////////////
#define MAX_REQUESTS 1		// MAXIMUM NUMBER OF REQUESTS TO SEND

///////////////////////////////////////////////////////////////////////////////////////////////////
class PieceRequestor
{
	private:
		bool terminated;
		vector<Piece*> &pieces;
		vector<void*> &peers;

		vector<bool> pieceProcessing;
		mutex requestMtx;
		vector<unsigned char *> requestedPeerIds;

	public:
		PieceRequestor(vector<Piece*> &, vector<void*> &);
		void startPieceRequestor();
		void stopPieceRequestor() { terminated = true; }

		bool selectRandomUnavailableUnprocessedPiece(size_t&, size_t&, size_t&, unsigned char **);
		bool allPiecesAvailable();
		bool unavailablePieceIsServicable();
		//bool selectRarestUnavailableUnprocessedPiece(size_t&, size_t&);
		void waitForGoAhead();
		void signalGoAhead(void *);
		bool selectServicablePeer(size_t, unsigned char **);
		void writeDirtyPiecesToDisk();

		void setPieceProcessing(size_t);			// piece processing = true
		void resetPieceProcessing(size_t);			// piece processing = false
		bool isPieceProcessing(size_t);				// is piece processing

		bool isPieceAvailable(size_t pieceId) { return pieces.size() > pieceId && pieces[pieceId]->isValid(); }	// is piece available & valid
		bool isPieceDirty(size_t pieceId) { return pieces.size() > pieceId && pieces[pieceId]->isDirty(); }		// is piece dirty
};

#endif /* PIECEREQUESTOR_HPP_ */
///////////////////////////////////////////////////////////////////////////////////////////////////
