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

#include "Monitor.h"
#include "PathManager.h"
#include "Environment.h"
#include "PathFinder.h"
#include "RopeCluster.h"
#include "Entity.h"
#include <iostream>

Monitor::Monitor(PathFinder *pf, std::vector<Instance *> list)
{
	_pf = pf;
	_instances = list;
	for (size_t i = 0; i < _instances.size(); i++)
	{
		_inst2Index[_instances[i]] = i;
	}

	{
		std::unique_lock<std::mutex> lock(_matrixMutex);
		setupMatrix(&_matrix, _instances.size(), _instances.size());
	}

	setup();
}

void Monitor::setup()
{
	for (Instance *i : _instances)
	{
		for (Instance *j : _instances)
		{
			if (i == j)
			{
				continue;
			}
			
			_results[i][j] = RouteResults{};
		}
	}

}

PCA::Matrix &Monitor::matrix()
{
	std::unique_lock<std::mutex> lock(_matrixMutex);
	for (int i = 0; i < _instances.size(); i++)
	{
		for (int j = 0; j < _instances.size(); j++)
		{
			if (i == j)
			{
				continue;
			}
			
			Instance *inst = _instances[i];
			Instance *jnst = _instances[j];
			
			RouteResults &rr = _results[inst][jnst];
			if (rr.valid)
			{
				float val = 1 - rr.linearity;
				_matrix[i][j] = val;
			}
			else if (rr.passes == 0)
			{
				_matrix[i][j] = -1;
			}
			else
			{
				_matrix[i][j] = NAN;
			}
		}
	}
	
	return _matrix;
}

void Monitor::addValidation(Instance *first, Instance *second, 
                            bool valid, float linearity)
{
	std::unique_lock<std::mutex> lock(_matrixMutex);
	_results[first][second].passes++;
	_results[first][second].valid = valid;
	_results[first][second].linearity = linearity;
	lock.unlock();
	
	matrix();
}

void Monitor::setStatus(Instance *first, Instance *second, TaskType type)
{
	_results[first][second].status = type;
	StatusInfo info{_inst2Index[first], _inst2Index[second], type};

	sendResponse("status", &info);
}

void Monitor::updatePath(Instance *first, Instance *second, Path *path)
{
	std::unique_lock<std::mutex> lock(_mapMutex);
	_paths.insert(path);
	RouteResults &rr = _results[first][second];
	if (rr.path)
	{
		_paths.erase(rr.path);
		delete rr.path;
	}
	rr.path = path;
//	path->setContributeToSVD(true);
	Environment::pathManager()->insertOrReplace(*path);
}

int Monitor::passesForPath(Instance *first, Instance *second)
{
	std::unique_lock<std::mutex> lock(_mapMutex);
	RouteResults &rr = _results[first][second];
	return rr.passes;
}

Path *Monitor::existingPath(Instance *first, Instance *second)
{
	std::unique_lock<std::mutex> lock(_mapMutex);
	RouteResults &rr = _results[first][second];
	return rr.path;
}

