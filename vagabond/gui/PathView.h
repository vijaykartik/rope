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

#ifndef __vagabond__PathView__
#define __vagabond__PathView__

#include <thread>
#include <atomic>
#include <vagabond/gui/elements/SimplePolygon.h>

class MetadataGroup;
template <class DG>
class ClusterSVD;

class Path;

class PathView : public SimplePolygon
{
public:
	PathView(Path &path, ClusterSVD<MetadataGroup> *cluster);
	virtual ~PathView();
	
	Path *path()
	{
		return &_path;
	}

	void populate();
	
	bool needsUpdate();
	
	bool isPopulated() const
	{
		return _populated;
	}
private:
	ClusterSVD<MetadataGroup> *_cluster = nullptr;
	
	Path &_path;
	std::atomic<bool> _populated{false};
	int _version = -1;
};

#endif
