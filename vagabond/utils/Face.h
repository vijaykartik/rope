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

#ifndef __vagabond__Face__
#define __vagabond__Face__

#include <nlohmann/json.hpp>
using nlohmann::json;

#include <vagabond/utils/AcquireCoord.h>
#include <iostream>
#include <mutex>
#include <map>
#include <set>
#include <float.h>
#include "svd/PCA.h"
#include "Point.h"


template <int N, unsigned int D, typename Type>
class SharedFace;

template <class Type>
class Mappable
{
public:
	virtual Coord::Interpolate<Type> interpolate_function() = 0;

	virtual Type interpolate_subfaces(const std::vector<float> &cart 
	                                  = {}) const = 0;

	virtual std::vector<float> 
	point_to_barycentric(const std::vector<float> &m) const = 0;

	virtual void
	point_to_barycentric(const Coord::Get &coord, std::vector<float> &w) const = 0;

	virtual std::vector<float> 
	barycentric_to_point(const std::vector<float> &b) const = 0;

	virtual bool point_in_bounds(const std::vector<float> &m) const = 0;
	
	virtual size_t version() = 0;
	virtual void invalidate() {};
	virtual bool valid() {return true;};
	virtual int n() = 0;

	virtual ~Mappable()
	{

	}
};

template <unsigned int D, class Type>
class MappedInDim : public Mappable<Type>
{
public:
	virtual bool hasPoint(const SharedFace<0, D, Type> &point) const = 0;
	virtual std::vector<SharedFace<0, D, Type> *> points() = 0;

	virtual ~MappedInDim()
	{

	}
};

template <int N, unsigned int D, typename Type>
class Interpolatable : public MappedInDim<D, Type>
{
public:
	Interpolatable()
	{

	}
	
	virtual ~Interpolatable()
	{

	}

	virtual SharedFace<N-1, D, Type> *
	faceExcluding(const SharedFace<0, D, Type> *point) const
	{
		return nullptr;
	}

	virtual Coord::Interpolate<Type> interpolate_function()
	{
		if (!_interpolate)
		{
			recalculate_interpolation();
		}

		return _interpolate;
	}
	
	virtual std::vector<float> middle_of_face()
	{
		float pc = pointCount();
		std::vector<float> bc(pc, 1 / pc);
		return barycentric_to_point(bc);
	}

	virtual Type interpolate_subfaces(const std::vector<float> &cart = {}) const
	{
		std::vector<float> weights = point_to_barycentric(cart);
		std::vector<float> inverse(weights.size(), 1);
		
		for (int i = 0; i < weights.size(); i++)
		{
			for (int j = 0; j < weights.size(); j++)
			{
				if (i == j)
				{
					continue;
				}

				inverse[i] *= weights[j];
			}
		}

		Type total{}; float count = 0;

		for (int i = 0; i < pointCount(); i++)
		{
			Type init = point(i)->value();
			const float &fw = inverse[i];

			SharedFace<N - 1, D, Type> *ls = faceExcluding(point(i));

			if (!ls) 
			{ 
				continue;
			}

			Type end = ls->value_for_point(cart);
			const float &w = weights[i]; // weight of point
			const float &r = 1 - weights[i]; // weight of simplex
			
			Type res = (init * w + end * r);
			total += res * fw;
			count += fw;
		}
		
		return total / count;
	}

	virtual Type value_for_point(const std::vector<float> &cart)
	{
		return interpolate_subfaces(cart);
	}
	
	struct PointStuff
	{
		const SharedFace<0, D, Type> *point;
		const SharedFace<N - 1, D, Type> *face;
		Coord::Interpolate<Type> interp_face;
	};

	Type interpolate_for_n_equal_1(const Coord::Get &coord) const
	{
		std::vector<float> weights(pointCount());
		this->point_to_barycentric(coord, weights);

		Type total{}; float count = 0;
		for (int i = 0; i < pointCount(); i++)
		{
			Type init = point(i)->value();
			Type end = point(1 - i)->value();
			const float &w = weights[i]; // weight of point
			const float &r = 1 - w; // weight of simplex

			Type res = (init * w + end * r);
			total += res * w;
			count += w;
		}

		return total / count;
	}

	void interpolate_for_n_equal_1(std::vector<PointStuff> &prep)
	{
		Coord::Interpolate<Type> func = 
		[this](const Coord::Get &coord)
		{
			return interpolate_for_n_equal_1(coord);
		};
		
		_interpolate = func;
	}

	void interpolate_for_n_gt_1(std::vector<PointStuff> &prep)
	{
		Coord::Interpolate<Type> func = 
		[this, prep](const Coord::Get &coord)
		{
			std::vector<float> weights(pointCount());
			this->point_to_barycentric(coord, weights);
			std::vector<float> inverse(weights.size(), 1);

			for (int i = 0; i < pointCount(); i++)
			{
				for (int j = 0; j < pointCount(); j++)
				{
					if (i == j)
					{
						continue;
					}

					inverse[i] *= weights[j];
				}
			}

			Type total{}; float count = 0;
			for (int i = 0; i < prep.size(); i++)
			{
				const PointStuff &stuff = prep[i];

				Type init = stuff.point->value();
				const float &fw = inverse[i];
				Type end{};
				if (N == 2)
				{
					end = stuff.face->interpolate_for_n_equal_1(coord);
				}
				else
				{
					end = stuff.interp_face(coord);
				}
				const float &w = weights[i]; // weight of point
				const float &r = 1 - w; // weight of simplex

				Type res = (init * w + end * r);
				total += res * fw;
				count += fw;
			}

			return total / count;
		};

		_interpolate = func;
	}
	
	void recalculate_interpolation()
	{
		std::vector<PointStuff> prep;
		for (int i = 0; i < this->pointCount(); i++)
		{
			PointStuff stuff;
			stuff.point = this->point(i);

			SharedFace<N - 1, D, Type> *ls = this->faceExcluding(stuff.point);
			if (!ls) { continue; }

			stuff.face = ls;
			stuff.interp_face = ls->interpolate_function();
			prep.push_back(stuff);
		}

		if (N > 1)
		{
			interpolate_for_n_gt_1(prep);
		}
		else if (N == 1)
		{
			interpolate_for_n_equal_1(prep);
		}
	}
	
	void get_inversion()
	{
		PCA::SVD svd;
		setupSVD(&svd, D+1, pointCount());

		for (size_t row = 0; row < D; row++)
		{
			for (size_t col = 0; col < pointCount(); col++)
			{
				svd.u[row][col] = point(col)->scalar(row);
			}
		}

		for (size_t col = 0; col < pointCount(); col++)
		{
			svd.u[D][col] = 1;
		}
		
		invertSVD(&svd);
		_tr = transpose(&svd.u);
		
		freeSVD(&svd);
	}

	virtual void bounds(std::vector<float> &min, std::vector<float> &max) const
	{
		min = std::vector<float>(D, FLT_MAX);
		max = std::vector<float>(D, -FLT_MAX);
		for (int i = 0; i < pointCount(); i++)
		{
			std::vector<float> p = *point(i);
			for (int i = 0; i < p.size(); i++)
			{
				if (min[i] > p.at(i)) { min[i] = p.at(i); }
				if (max[i] < p.at(i)) { max[i] = p.at(i); }
			}
		}
	}

	virtual bool point_in_bounds(const std::vector<float> &m) const
	{
		std::vector<float> min, max;
		bounds(min, max);
		for (int j = 0; j < m.size(); j++)
		{
			if (m[j] < min[j] || m[j] > max[j])
			{
				return false;
			}
		}

		return true;
	}

	virtual std::vector<float> 
	barycentric_to_point(const std::vector<float> &b) const
	{
		std::vector<float> vec(D, 0);
		
		for (size_t j = 0; j < pointCount(); j++)
		{
			std::vector<float> p = *point(j);
			for (size_t i = 0; i < D; i++)
			{
				vec[i] += b[j] * p[i];
			}
		}

		return vec;
	}

	virtual std::vector<float> 
	point_to_barycentric(const std::vector<float> &m) const
	{
		std::vector<float> weights(pointCount());
		point_to_barycentric(Coord::fromVector(m), weights);
		return weights;
	}

	virtual void
	point_to_barycentric(const Coord::Get &coord,
	                     std::vector<float> &weights) const
	{
		float vec[D + 1];
		
		for (size_t i = 0; i < D; i++)
		{
			vec[i] = coord(i);
		}

		vec[D] = 1;
		
		multMatrix(_tr, vec, &weights[0]);
	}

private:
	virtual size_t pointCount() const = 0;
	virtual const SharedFace<0, D, Type> *point(int idx) const = 0;
protected:
	PCA::Matrix _tr;
	Coord::Interpolate<Type> _interpolate;
};

template <unsigned int D, typename Type>
class Interpolatable<0, D, Type> : public Mappable<Type>
{
public:
	virtual Type exact_value() const = 0;
	Type interpolate_for_n_equal_1(const Coord::Get &coord) const
	{
		return exact_value();
	}

	virtual void changed() {};

	virtual Type interpolate_subfaces(const std::vector<float> &cart = {}) const
	{
		return exact_value();
	}
	
	virtual Type value_for_point(const std::vector<float> &cart)
	{
		return exact_value();
	}
};

template <int N, unsigned int D, typename Type>
class SharedFace : public Interpolatable<N, D, Type>
{
public:
	typedef SharedFace<N - 1, D, Type> LowerFace;
	typedef SharedFace<N, D, Type> SameFace;
	
	virtual ~SharedFace<N, D, Type>() {}
	
	virtual int n()
	{
		return N;
	}

	SharedFace(LowerFace *face, SharedFace<0, D, Type> &point) :
	SharedFace<N, D, Type>(*face, point)
	{
		this->changed();
	}

	SharedFace(const SameFace &face)
	{
		_points = face._points;
		
		for (auto &lf : face.c_subs())
		{
			LowerFace *nf = new LowerFace(*lf);
			this->subs().push_back(nf);
		}
		this->changed();
	}

	SharedFace(LowerFace &face, SharedFace<0, D, Type> &point)
	{
		this->subs().push_back(&face);
		_points = face.points();
		
		for (auto &lf : face.c_subs())
		{
			LowerFace *nf = new LowerFace(lf, point);
			this->subs().push_back(nf);
		}
		
		this->_points.push_back(&point);
		this->changed();
	}
	
	void add_to_bins(std::map<int, std::set<MappedInDim<D, Type> *>> &bins)
	{
		bins[N].insert(this);

		for (int i = 0; i < subs().size(); i++)
		{
			subs()[i]->add_to_bins(bins);
		}
	}

	static SharedFace<N, D, Type> *make_next(std::vector<SharedFace<0, D, Type> *> src)
	{
		if (src.size() <= N)
		{
			throw std::runtime_error("making face from too few vertices");
		}
		SharedFace<N, D, Type> *combine;
		combine = new SharedFace<N, D, Type>(*LowerFace::make_next(src), *src[N]);
		return combine;
	}
	
	virtual void invalidate()
	{
		_valid = false;
	}

	virtual bool valid() {return _valid;};

	virtual void changed()
	{
		_version++;
		this->get_inversion();
		
		for (int i = 0; i < subs().size(); i++)
		{
			subs()[i]->changed();
		}

		this->recalculate_interpolation();
	}
	
	virtual LowerFace *faceExcluding(const SharedFace<0, D, Type> *point) const
	{
		for (auto &f : c_subs())
		{
			if (!f->hasPoint(*point))
			{
				return f;
			}
		}
		
		return nullptr;
	}

	virtual size_t version()
	{
		return _version;
	}
	
	virtual std::vector<float> cartesian_circumcenter(float *radius = nullptr)
	{
		PCA::SVD mat; PCA::Matrix vect;
		setupSVD(&mat, pointCount(), D);
		setupMatrix(&vect, pointCount(), 1);
		std::vector<float> result(D);

		for (int i = 0; i < pointCount(); i++)
		{
			int n = (i == pointCount() ? 0 : i);
			int m = (i == pointCount() - 1 ? 1 : n + 1);

			Point<D, Type> *vm = _points[m];
			Point<D, Type> *vn = _points[n];
			Point<D, Type> diff = *vm - *vn;
			float l = vm->sqlength() - vn->sqlength();
			vect[i][0] = l;
			
			for (int j = 0; j < D; j++)
			{
				mat.u[i][j] = diff[j] * 2;
			}
		}
		
		invertSVD(&mat);
		PCA::Matrix tr = transpose(&mat.u);
		multMatrix(tr, vect[0], &result[0]);

		freeSVD(&mat);
		freeMatrix(&vect);
		
		if (pointCount() > 0 && radius != nullptr)
		{
			std::vector<float> v0 = *_points[0];
			float sum = 0;
			for (size_t i = 0; i < v0.size(); i++)
			{
				float add = (v0[i] - result[i]);
				sum += add * add;
			}
			*radius = sqrt(sum);
		}
		
		return result;
	}
	
	virtual const SharedFace<0, D, Type> *point(int idx) const
	{
		return _points[idx];
	}
	
	virtual SharedFace<0, D, Type> &v_point(int idx) const
	{
		return *_points[idx];
	}

	virtual bool hasPoint(const SharedFace<0, D, Type> &point) const
	{
		for (SharedFace<0, D, Type> *p : _points)
		{
			if (p == &point)
			{
				return true;
			}
		}
		
		return false;
	}

	SharedFace<0, D, Type> *point_not_in(SameFace *other)
	{
		for (SharedFace<0, D, Type> *p : _points)
		{
			if (!other->hasPoint(*p))
			{
				return p;
			}
		}

		return nullptr;
	}
	
	std::set<SharedFace<0, D, Type> *> shared_points(SameFace *other)
	{
		std::set<SharedFace<0, D, Type> *> ps;

		for (SharedFace<0, D, Type> *p : _points)
		{
			bool has = other->hasPoint(*p);
			if (has)
			{
				ps.insert(p);
			}
		}

		for (SharedFace<0, D, Type> *p : other->_points)
		{
			bool has = hasPoint(*p);
			if (has)
			{
				ps.insert(p);
			}
		}
		
		return ps;
	}
	
	void swap_point(SharedFace<0, D, Type> *out,
	                SharedFace<0, D, Type> *in)
	{
		for (LowerFace *face : _subs)
		{
			face->swap_point(out, in);
		}

		for (auto it = _points.begin(); it != _points.end(); it++)
		{
			if (*it == out)
			{
				*it = in;
			}
		}
	}
	
	int shared_point_count(SameFace *other)
	{
		int total = 0;

		for (SharedFace<0, D, Type> *p : _points)
		{
			for (SharedFace<0, D, Type> *q : other->_points)
			{
				total += (p == q) ? 1 : 0;
			}
		}
		
		return total;
	}

	virtual std::vector<SharedFace<0, D, Type> *> points()
	{
		return _points;
	}

	virtual size_t pointCount() const
	{
		return _points.size();
	}
	
	size_t faceCount()
	{
		return _subs.size();
	}

	std::vector<LowerFace *> &subs() 
	{
		return _subs;
	}

	const std::vector<LowerFace *> &c_subs() const
	{
		return _subs;
	}

	friend std::ostream &operator<<(std::ostream &ss, const SameFace &f)
	{
		ss << "Face<" << N << ">, " << D << " dimensions" << std::endl;
		int i = 0;
		for (auto &lf : f.c_subs())
		{
			ss << i << ": " <<  *lf;
			i++;
		}
		return ss;
	}

	friend void to_json(json &j, const SharedFace<N, D, Type> &face);
	friend void from_json(const json &j, SharedFace<N, D, Type> &face);
protected:
	SharedFace() {};

	std::vector<LowerFace *> _subs;
	std::vector<SharedFace<0, D, Type> *> _points;

	size_t _version = 0;
	bool _valid = true;
	int _n = N;
	int _d = D;
};

template <unsigned int D, typename Type>
class SharedFace<1, D, Type> : public Interpolatable<1, D, Type>
{
public:
	typedef SharedFace<1, D, Type> SameFace;

	SharedFace(SharedFace<0, D, Type> *face, SharedFace<0, D, Type> &point) :
	SharedFace<1, D, Type>(*face, point) {}

	SharedFace<1, D, Type>() {}
	virtual ~SharedFace<1, D, Type>() {}

	virtual int n()
	{
		return 1;
	}

	virtual size_t version()
	{
		return _version;
	}

	SharedFace(SharedFace<0, D, Type> &face, SharedFace<0, D, Type> &point)
	{
		_subs.push_back(&face);
		_subs.push_back(&point);
		_points = _subs;
	}

	void add_to_bins(std::map<int, std::set<MappedInDim<D, Type> *>> &bins)
	{
		bins[1].insert(this);
		for (SharedFace<0, D, Type> *f : c_subs())
		{
			bins[0].insert(f);
		}
	}

	static SharedFace<1, D, Type> *
	make_next(std::vector<SharedFace<0, D, Type> *> src)
	{
		SharedFace<1, D, Type> *line = new SharedFace<1, D, Type>(*src[0], *src[1]);
		return line;
	}

	SharedFace(const SameFace &face)
	{
		_points = face._points;
		_subs = _points;
	}

	virtual void changed()
	{
		_version++;
		this->get_inversion();
	}

	size_t pointCount() const
	{
		return _points.size();
	}
	
	virtual std::vector<SharedFace<0, D, Type> *> points()
	{
		return _points;
	}
	
	virtual const SharedFace<0, D, Type> *point(int idx) const
	{
		return _points[idx];
	}

	virtual SharedFace<0, D, Type> &v_point(int idx) const
	{
		return *_points[idx];
	}

	virtual SharedFace<0, D, Type> *faceExcluding(const SharedFace<0, D, Type> 
	                                              *point) const
	{
		for (SharedFace<0, D, Type> *f : c_subs())
		{
			if (f != point)
			{
				return f;
			}
		}
		
		return nullptr;
	}

	void swap_point(SharedFace<0, D, Type> *out,
	                SharedFace<0, D, Type> *in)
	{
		for (int i = 0; i < _subs.size(); i++)
		{
			if (_subs[i] == out)
			{
				_subs[i] = in;
			}
		}

		for (auto it = _points.begin(); it != _points.end(); it++)
		{
			if (*it == out)
			{
				*it = in;
			}
		}
	}

	size_t faceCount() const
	{
		return _subs.size();
	}

	const std::vector<SharedFace<0, D, Type> *> &c_subs() const
	{
		return _subs;
	}

	std::vector<SharedFace<0, D, Type> *> &subs() 
	{
		return _subs;
	}

	virtual bool hasPoint(const SharedFace<0, D, Type> &point) const
	{
		for (SharedFace<0, D, Type> *p : _points)
		{
			if (p == &point)
			{
				return true;
			}
		}
		
		return false;
	}


	friend std::ostream &operator<<(std::ostream &ss, const SameFace &f)
	{
		ss << "Face<" << 1 << ">, " << D << " dimensions" << std::endl;
		int i = 0;
		for (auto &lf : f.c_subs())
		{
			ss << i << ": " <<  *lf;
			i++;
		}
		return ss;
	}

	friend void to_json(json &j, const SharedFace<1, D, Type> &face);
	friend void from_json(const json &j, SharedFace<1, D, Type> &face);
private:
	std::vector<SharedFace<0, D, Type> *> _subs;
	std::vector<SharedFace<0, D, Type> *> _points;

	int _n = 0;
	int _d = D;
	size_t _version = 0;
};

template <unsigned int D, typename Type>
class SharedFace<0, D, Type> : public Interpolatable<0, D, Type>, 
public Point<D, Type>, public MappedInDim<D, Type>
{
public:
	SharedFace() : Point<D, Type>() {};
	SharedFace(const std::vector<float> &p, Type value = Type{}) :
	Point<D, Type>(p, value) {}
	SharedFace(float *p, Type val) : Point<D, Type>(p, val) {};
	SharedFace(Point<D, Type> &p) : Point<D, Type>(p) {};
	SharedFace(SharedFace<0, D, Type> &face, SharedFace<0, D, Type> &other) {};

	virtual int n()
	{
		return 0;
	}

	virtual size_t version()
	{
		return 0;
	}

	virtual bool point_in_bounds(const std::vector<float> &m) const
	{
		return false;
	}

	virtual Coord::Interpolate<Type> interpolate_function()
	{
		Coord::Interpolate<Type> ret;
		ret = [this](const Coord::Get &)
		{
			return this->value();
		};
		return ret;
	}

	virtual Type interpolate_subfaces(const std::vector<float> &cart = {}) const
	{
		return exact_value();
	}

	virtual std::vector<float> 
	barycentric_to_point(const std::vector<float> &b) const
	{
		std::vector<float> p = *this;
		return p;
	}

	virtual void
	point_to_barycentric(const Coord::Get &coord,
	                     std::vector<float> &weights) const
	{

	}

	virtual std::vector<float> 
	point_to_barycentric(const std::vector<float> &m) const
	{
		return std::vector<float>(1, 0);
	}

	virtual ~SharedFace<0, D, Type>() {}

	virtual Type exact_value() const
	{
		return this->value();
	}

	virtual size_t pointCount() const
	{
		return 0;
	}

	size_t faceCount()
	{
		return 0;
	}

	virtual bool hasPoint(const SharedFace<0, D, Type> &point) const
	{
		return (this == &point);
	}


	virtual std::vector<SharedFace<0, D, Type> *> points()
	{
		return std::vector<SharedFace<0, D, Type> *>();
	}
	
	const std::vector<SharedFace<0, D, Type> *> c_subs() const
	{
		return std::vector<SharedFace<0, D, Type> *>();
	}

	virtual const SharedFace<0, D, Type> *point(int idx) const
	{
		return nullptr;
	}

private:
	int _n = 0;
	int _d = D;
	
};

#endif
