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
#define MAX_REQUESTS 1			// MAXIMUM NUMBER OF REQUESTS TO SEND
#define REQUEST_TIMEOUT_MILLIS 5000	// MAXIMUM TIME TO WAIT FOR A PIECE RESPONSE

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
		vector<size_t> requestTimestamps;
		vector<size_t> requestedPieceIds;
		vector<size_t> requestedBlockIds;

		bool updateSts;
		bool firstRequestSent;
		size_t firstRequestBegin;
		size_t runTime;

	public:
		PieceRequestor(vector<Piece*> &, vector<void*> &);
		void startPieceRequestor();
		void stopPieceRequestor() { terminated = true; }
		string status(size_t &totalBlocksCompleted, size_t &totalBlocks);
		double getRunTime() { return runTime; }	// Milliseconds

		bool selectRandomUnavailableUnprocessedPiece(size_t&, size_t&, size_t&, unsigned char **);
		bool allPiecesAvailable();
		bool unavailablePieceIsServicable();
		//bool selectRarestUnavailableUnprocessedPiece(size_t&, size_t&);
		void waitForGoAhead();
		void signalTimeout();
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
