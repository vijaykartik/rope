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

#include "engine/workers/ThreadExtractsBondPositions.h"
#include "BondSequenceHandler.h"
#include "engine/SurfaceAreaHandler.h"
#include "engine/MapTransferHandler.h"
#include "engine/ForceFieldHandler.h"
#include "engine/MechanicalBasis.h"
#include "engine/SolventHandler.h"
#include "BondCalculator.h"
#include "engine/PointStoreHandler.h"
#include "BondSequence.h"
#include <iostream>
#include <algorithm>

ThreadExtractsBondPositions::ThreadExtractsBondPositions(BondSequenceHandler *h)
: ThreadWorker()
{
	_seqHandler = h;
	_finish = false;
}

void ThreadExtractsBondPositions::extractVector(Job *job, BondSequence *seq)
{
	Result *r = job->result;

	const AtomPosList &apl = seq->extractVector();
	r->apl = apl;
}

void ThreadExtractsBondPositions::extractPositions(Job *job, BondSequence *seq)
{
	Result *r = job->result;

	const AtomPosMap &aps = seq->extractPositions();
	r->aps = aps;
}

void ThreadExtractsBondPositions::transferToSurfaceHandler(Job *job,
                                                           BondSequence *seq)
{
	AtomPosMap aps = seq->extractPositions();
	SurfaceAreaHandler *handler = _seqHandler->calculator()->surfaceHandler();
	
	handler->sendJobForCalculation(job, aps);
	timeEnd();
	cleanupSequence(job, seq);
}

void ThreadExtractsBondPositions::transferToForceFields(Job *job,
                                                        BondSequence *seq)
{
	const AtomPosMap &aps = seq->extractPositions();
	ForceFieldHandler *ffHandler = _seqHandler->calculator()->forceFieldHandler();
	
	ffHandler->atomMapToForceField(job, aps);
	timeEnd();
	cleanupSequence(job, seq);
}

void ThreadExtractsBondPositions::calculateDeviation(Job *job, BondSequence *seq)
{
	Result *r = job->result;
	r->deviation = seq->calculateDeviations();
}

void ThreadExtractsBondPositions::transferToMaps(Job *job, BondSequence *seq)
{
	std::vector<BondSequence::ElePos> epos = seq->extractForMap();
	_pointHandler->loadMixedPositions(job, epos);
	
	if (job->requests & JobSolventMask)
	{
		const AtomPosMap &aps = seq->extractPositions();
		_solventHandler->loadAtomPosMap(aps);
	}

	timeEnd();
	cleanupSequence(job, seq);
}

void ThreadExtractsBondPositions::start()
{
	SequenceState state = SequencePositionsReady;
	do
	{
		BondSequence *seq = _seqHandler->acquireSequence(state);
		
		if (seq == nullptr)
		{
			break;
		}
		
		timeStart();

		Job *job = seq->job();

		Result *r = nullptr;

		if (job->result)
		{
			r = job->result;
		}
		else
		{
			if (r == nullptr)
			{
				r = new Result();
				r->setFromJob(job);
			}
		}

		if (job->requests & JobCalculateDeviations)
		{
			calculateDeviation(job, seq);
		}
		if (job->requests & JobExtractPositions)
		{
			extractPositions(job, seq);
		}
		if (job->requests & JobPositionVector)
		{
			extractVector(job, seq);
		}

		/* if the solvent area is requested, this will be done before
		 * force field measurement, and passed onto force field later by
		 * ThreadSurfacer */
		if (job->requests & JobSolventSurfaceArea)
		{
			transferToSurfaceHandler(job, seq);
			continue; // loses control of bond sequence
		}
		else if (job->requests & JobScoreStructure)
		{
			transferToForceFields(job, seq);
			continue; // loses control of bond sequence
		}
		if (job->requests & JobCalculateMapSegment ||
		    job->requests & JobMapCorrelation)
		{
			transferToMaps(job, seq);
			continue; // loses control of bond sequence
		}

		cleanupSequence(job, seq);
			
		returnResult(job);

		timeEnd();

	}
	while (!_finish);
}

void ThreadExtractsBondPositions::cleanupSequence(Job *job, BondSequence *seq)
{
	seq->cleanUpToIdle();
}

void ThreadExtractsBondPositions::returnResult(Job *job)
{
	BondCalculator *calc = _seqHandler->calculator();
	Result *r = job->result;
	job->destroy();
	calc->submitResult(r);
}
