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

#ifndef __vagabond__SandboxView__
#define __vagabond__SandboxView__

#include <vagabond/gui/Display.h>
#include <vagabond/utils/Mapping.h>
#include <vagabond/core/Responder.h>
#include <thread>

class MappingToMatrix;
class SpecificNetwork;
class MatrixPlot;
class Network;

class SandboxView : public Display, public Responder<SpecificNetwork>
{
public:
	SandboxView(Scene *prev);
	virtual ~SandboxView();

	void makeTriangles();
	virtual void setup();
private:
	void makeMapping();
	bool _editing = false;

	Mapped<float> *_mapped = nullptr;
	MappingToMatrix *_mat2Map = nullptr;
	MatrixPlot *_plot = nullptr;
	Network *_network = nullptr;
	SpecificNetwork *_specified = nullptr;
	std::mutex _mutex;

	MatrixPlot *_distances = nullptr;
	PCA::Matrix _distMat;
};

#endif
