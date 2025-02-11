// vagabond
// Copyright (C) 2022 Helen Ginn
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

#include "engine/MapSumHandler.h"
#include "ElementSegment.h"
#include "engine/MapTransferHandler.h"
#include "engine/CorrelationHandler.h"
#include "engine/workers/ThreadMapSummer.h"
#include "AtomSegment.h"
#include "Job.h"

MapSumHandler::MapSumHandler(BondCalculator *calculator)
{
	_calculator = calculator;
	_mapHandler = _calculator->mapHandler();
}

MapSumHandler::~MapSumHandler()
{
	delete _template;
	finish();
}

void MapSumHandler::createSegments()
{
	for (size_t i = 0; i < _mapCount + 1; i++)
	{
		AtomSegment *seg = new AtomSegment();

		if (_mapHandler->elementCount() > 0)
		{
			seg->getDimensionsFrom(*_mapHandler->segment(0));
			seg->setStatus(FFT<Density>::Real);
		}
		
		/* in order to calculate the plan once, rather than via competing
		 *	threads later */
		if (i == 0)
		{
			_template = new AtomMap(*seg);
		}

		_mapPool.pushObject(seg);
	}
}

void MapSumHandler::setup()
{
	createSegments();
}

void MapSumHandler::prepareThreads()
{
	for (size_t i = 0; i < _threads + 1; i++)
	{
		/* several calculators */
		ThreadMapSummer *worker = new ThreadMapSummer(this);
		worker->setMapHandler(_mapHandler);
		std::thread *thr = new std::thread(&ThreadMapSummer::start, worker);

		_mapPool.addWorker(worker, thr);
	}

}

void MapSumHandler::start()
{
	prepareThreads();
}

MapSumHandler::MapJob *MapSumHandler::acquireMapJob(Job *job)
{
	int ticket = job->ticket;
	std::lock_guard<std::mutex> lock(_ticketHandout);

	if (_ticketMap.count(ticket))
	{
		return _ticketMap[ticket];
	}

	AtomSegment *seg;
	_mapPool.acquireObject(seg);

	MapJob *mj = new MapJob();
	mj->segment = seg;
	mj->job = job;

	_ticketMap[ticket] = mj;
	
	return mj;
}

void MapSumHandler::transferElementSegment(ElementSegment *segment)
{
	_segmentPool.pushObject(segment);
}

ElementSegment *MapSumHandler::acquireElementSegment(MapJob *&mj)
{
	ElementSegment *seg = nullptr;
	_segmentPool.acquireObject(seg);

	if (seg == nullptr)
	{
		return nullptr;
	}

	Job *j = seg->job();
	mj = acquireMapJob(j);

	return seg;
}

void MapSumHandler::returnSegment(AtomSegment *segment)
{
	segment->clear();
	_mapPool.pushObject(segment);
}

void MapSumHandler::returnMiniJob(MapJob *mj)
{
	_handout.lock();
	mj->summed++;
	int val = mj->summed;
	
	if (_ticketMap.size() > 1 && false)
	{
		std::cout << "Open tickets: " << _ticketMap.size() << std::endl;

		std::map<int, MapJob *>::iterator it;
		for (it = _ticketMap.begin(); it != _ticketMap.end(); it++)
		{
			std::cout << "Ticket: " << it->first << " ";
			std::cout << "total: " << it->second->summed << std::endl;
		}
	}
	_handout.unlock();

	if (val < _mapHandler->elementCount())
	{
		return;
	}
	
	Job *job = mj->job;
	int ticket = job->ticket;

	_ticketHandout.lock();
	_ticketMap.erase(ticket);
	_ticketHandout.unlock();

	Result *r = job->result;
	if (r == nullptr)
	{
		r = new Result();
		r->setFromJob(job);
	}

	if (job->requests & JobCalculateMapSegment) 
	{
		r->map = new AtomMap(*_template);
		r->map->copyData(*mj->segment);
		
		if (!(job->requests & JobMapCorrelation))
		{
			returnSegment(mj->segment);
		}
	}
	
	if (job->requests & JobMapCorrelation)
	{
		_correlHandler->pushMap(mj->segment, job);
	}
	else
	{
		returnSegment(mj->segment);
		job->destroy();
		_calculator->submitResult(r);
	}

	delete mj;
}

void MapSumHandler::finish()
{
	_mapPool.finish();
	_segmentPool.finish();
}
