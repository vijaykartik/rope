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

#ifndef __vagabond__GuiRefls__
#define __vagabond__GuiRefls__

#include <vagabond/gui/elements/SimplePolygon.h>

class Diffraction;
class Icosahedron;

class GuiRefls : public SimplePolygon
{
public:
	GuiRefls();

	virtual void render(SnowGL *gl);
	void populateFromDiffraction(Diffraction *diffraction);
	virtual void extraUniforms();
	
	void setSlice(bool slice)
	{
		_slice = (slice ? 1 : -1);
	}
private:
	GLfloat _slice = -1;

	float _size = 10;
};

#endif
