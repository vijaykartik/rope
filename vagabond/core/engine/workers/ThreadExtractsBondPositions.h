// vagabond
// Copyright (C) 2019 Helen Ginn
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
// 
// Please email: vagabond @ hginn.co.uk for more details.

#ifndef __vagabond__ThreadExtractsBondPositions__
#define __vagabond__ThreadExtractsBondPositions__

#include "engine/workers/ThreadWorker.h"

class MapTransferHandler;
class PointStoreHandler;
class BondSequenceHandler;
class BondCalculator;
class BondSequence;
struct Job;
class MiniJobSeq;
class SolventHandler;

class ThreadExtractsBondPositions : public ThreadWorker
{
public:
	ThreadExtractsBondPositions(BondSequenceHandler *h);
	virtual ~ThreadExtractsBondPositions() {};

	virtual void start();
	
	virtual std::string type()
	{
		return "Bond position extractor";
	}

	void setSolventHandler(SolventHandler *handler)
	{
		_solventHandler = handler;
	}

	void setPointStoreHandler(PointStoreHandler *handler)
	{
		_pointHandler = handler;
	}

private:
	void extractVector(Job *job, BondSequence *seq);
	void extractPositions(Job *job, BondSequence *seq);
	void calculateDeviation(Job *job, BondSequence *seq);
	void transferToMaps(Job *job, BondSequence *seq);
	void updateMechanics(Job *job, BondSequence *seq);
	void transferToForceFields(Job *job, BondSequence *seq);
	void transferToSurfaceHandler(Job *job, BondSequence *seq);
	void transferToSolventMask(Job *job, BondSequence *seq);
	
	void cleanupSequence(Job *job, BondSequence *seq);
	void returnResult(Job *job);

	BondSequenceHandler *_seqHandler = nullptr;
	PointStoreHandler *_pointHandler = nullptr;
	SolventHandler *_solventHandler = nullptr;

};

#endif
