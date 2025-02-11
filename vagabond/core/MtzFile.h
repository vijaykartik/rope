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

#ifndef __vagabond__MtzFile__
#define __vagabond__MtzFile__

#include "File.h"

class Diffraction;
class ArbitraryMap;

class MtzFile : public File
{
public:
	MtzFile(std::string filename = "");
	
	void setMap(Diffraction *diffraction)
	{
		_map = diffraction;
	}

	void setMap(ArbitraryMap *map);

	std::string write_to_string(float max_res = -1);
	virtual File::Type cursoryLook();
	virtual void parse();
private:
	Diffraction *_map = nullptr;

};

#endif
