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

#include "OnPathBasis.h"
#include "Trajectory.h"

OnPathBasis::OnPathBasis() : ConcertedBasis()
{

}

float OnPathBasis::contributionForAxis(BondSequence *seq, int tidx, int axis, 
                                       const Coord::Get &coord) const
{
	if (seq == nullptr || axis != 0 
	    || _traj == nullptr) // iterating over non-zero axis
	{
		return ConcertedBasis::contributionForAxis(seq, tidx, axis, coord);
	}

	float weight = coord(axis);
	float angle = _traj->angleForFraction(weight, tidx);
	return angle;
}
