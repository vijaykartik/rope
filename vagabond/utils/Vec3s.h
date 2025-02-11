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

#ifndef __vagabond__Vec3s__
#define __vagabond__Vec3s__

#include "glm_json.h"
#include "Mapping.h"
#include "Point.h"

class Vec3s : public std::vector<glm::vec3>
{
public:
	Vec3s operator/(const float &other) const
	{
		Vec3s ret;
		for (const glm::vec3 &v : *this)
		{
			ret.push_back(v / other);
		}

		return ret;
	}

	Vec3s operator*(const float &other) const
	{
		Vec3s ret;
		for (const glm::vec3 &v : *this)
		{
			ret.push_back(v * other);
		}

		return ret;
	}

	Vec3s operator+(const Vec3s &other) const
	{
		Vec3s ret;
		for (size_t i = 0; i < size(); i++)
		{
			ret.push_back(this->at(i) + other.at(i));
		}

		return ret;
	}

	Vec3s operator-(const Vec3s &other) const
	{
		Vec3s ret;
		for (size_t i = 0; i < size(); i++)
		{
			ret.push_back(this->at(i) - other.at(i));
		}

		return ret;
	}

	Vec3s &operator*=(const float &other)
	{
		for (size_t i = 0; i < size(); i++)
		{
			(*this)[i] *= other;
		}

		return *this;
	}

	Vec3s &operator+=(const Vec3s &other)
	{
		resize(other.size());

		for (size_t i = 0; i < size(); i++)
		{
			(*this)[i] += other[i];
		}

		return *this;
	}
	
	friend std::ostream &operator<<(std::ostream &ss, const Vec3s &vs)
	{
		std::cout << "vec3s(" << vs.size() << ")" << std::endl;
		return ss;
	}
private:

};

inline void to_json(json &j, const Point<2, Vec3s> &point)
{
	std::vector<float> coords = point;
	Vec3s value = point.value();

	j["coords"] = coords;
	j["val"] = value;
}

inline void from_json(const json &j, Point<2, Vec3s> &point)
{
	if (j.count("coords"))
	{
		std::vector<float> coords = j.at("coords");
		point.set_vector(coords);
	}

	if (j.count("val"))
	{
		const Vec3s &vs = j.at("val");
		point.set_value(vs);
	}
}

inline void to_json(json &j, const Mapping<2, Vec3s> &map)
{
	typedef SharedFace<0, 2, Vec3s> HyperPoint;
	std::vector<HyperPoint> points;
	for (auto p : map._points)
	{
		points.push_back(*p);
	}
	j["points"] = points;

	for (int i = 0; i < map._mapped.size(); i++)
	{
		std::vector<int> tIndices;
		map.point_indices_for_face(i, tIndices);
		j["face"][i] = tIndices;
	}

}

inline void from_json(const json &j, Mapping<2, Vec3s> &map)
{
	typedef SharedFace<0, 2, Vec3s> HyperPoint;
	if (j.count("points"))
	{
		for (HyperPoint p : j.at("points"))
		{
			HyperPoint *hp = new HyperPoint(p);
			map.add_point(hp);
		}
	}
	
	if (j.count("face"))
	{
		for (std::vector<int> idxs : j.at("face"))
		{
			std::vector<HyperPoint *> points;
			
			for (const int &idx : idxs)
			{
				points.push_back(map.point(idx));
			}

			map.add_simplex(points);
		}
	}
	
	map.update();
}


#endif
